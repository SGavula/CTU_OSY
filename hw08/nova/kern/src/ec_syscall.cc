#include "ec.h"
#include "ptab.h"
#include "bits.h"
 
#define MAX_ADDRESS 0xBFFFF000
 
typedef enum {
    sys_print      = 1,
    sys_sum        = 2,
    sys_break      = 3,
    sys_thr_create = 4,
    sys_thr_yield  = 5,
} Syscall_numbers;
 

void deallocate_memory(mword lower_bound, mword upper_bound) {
    for(mword page_num = lower_bound; page_num < upper_bound; page_num += PAGE_SIZE) {
        // Get physical address from table
        mword phys_addr = Ptab::get_mapping(page_num);
        if(!phys_addr) {
            printf("Cannot get phys addr.\n");
            break;
        }
        mword frame_num = phys_addr & (~PAGE_MASK);
        //Convert physical address to virtual
        void* virt_addr = Kalloc::phys2virt(frame_num);
        bool mapping_inserted = Ptab::insert_mapping(page_num, 0, 0);
        // Mapping error
        if(!mapping_inserted) {
            printf("Not enough memory.\n");
            break;
        }
        Kalloc::allocator.free_page(virt_addr);
    }
}

bool allocate_memory(mword lower_bound, mword upper_bound) {
    for(mword page_num = lower_bound; page_num < upper_bound; page_num += PAGE_SIZE) {
        void *new_page_addr = Kalloc::allocator.alloc_page(1, Kalloc::FILL_0);
        // Handling allocation of page
        if (!new_page_addr) {
            // Allocation error
            // Deallocate allocated memory
            deallocate_memory(lower_bound, page_num);
            printf("Not enough memory. 1\n");
            return false;
        }
        //Convert virtual to physical address - from insert_mapping function
        mword new_frame_addr = Kalloc::virt2phys(new_page_addr);
        // Save physical address to page table
        bool mapping_inserted = Ptab::insert_mapping(page_num, new_frame_addr, Ptab::PRESENT | Ptab::RW | Ptab::USER);
        // Handle mapping
        if(!mapping_inserted) {
            // Mapping error
            deallocate_memory(lower_bound, page_num);
            // Free allocated page in current cycle of loop
            Kalloc::allocator.free_page(new_page_addr);
            printf("Not enough memory\n");
            return false;
        }
    }
    return true;
}
 
void reset_memory(mword address, mword current_break_page_num, char* start) {
    mword upper_reset = address < current_break_page_num ? address : current_break_page_num;
    for (char* mem = start; mem < reinterpret_cast<char*>(upper_reset); mem++) {
        // printf("Reset memory address: %#lx\n", mem);
        *mem = 0;
    }
}
 
void Ec::syscall_handler (uint8 a)
{
    // Get access to registers stored during entering the system - see
    // entry_sysenter in entry.S
    // r is the pointer to the object that stores the information about sys registers
    Sys_regs * r = current->sys_regs();
    Syscall_numbers number = static_cast<Syscall_numbers> (a);
 
    switch (number) {
        case sys_print: {
            // Tisk řetězce na sériovou linku
            char *data = reinterpret_cast<char*>(r->esi);
            unsigned len = r->edi;
            for (unsigned i = 0; i < len; i++)
                printf("%c", data[i]);
            break;
        }
        case sys_sum: {
            // Naprosto nepotřebné systémové volání na sečtení dvou čísel
            int first_number = r->esi;
            int second_number = r->edi;
            r->eax = first_number + second_number;
            break;
        }
        case sys_break: {
            mword address = r->esi;
             
            // If address is zero for printing and if adress = break current and do not need to allocate
            if(address == 0 || address == Ec::break_current) {
                //r->eax = return value
                r->eax = Ec::break_current;
                break;
            } else if(address < Ec::break_min || address > MAX_ADDRESS) {
                printf("OUT OF RANGE\n");
                // Return 0 error
                r->eax = 0;
                break;
            }else if(address > Ec::break_current) {
                // ALOCATE PAGES
                // Get number from virtual addres
                mword current_break_page_num = align_up(Ec::break_current, PAGE_SIZE);
                mword upper_bound =  align_up(address, PAGE_SIZE);
                 
                // Memory reseting
                reset_memory(address, current_break_page_num, reinterpret_cast<char*>(Ec::break_current));

                // Creating pages
                bool memory_successfuly_allocated = allocate_memory(current_break_page_num, upper_bound);

                // return current break page
                if(memory_successfuly_allocated == true) {
                    r->eax = Ec::break_current;
                    Ec::break_current = address;
                }else {
                    r->eax = 0;
                }
                break;
            }else {
                // DEALLOCATE PAGES
                // Get number from virtual addres
                mword current_break_page_num = align_up(Ec::break_current, PAGE_SIZE);
                mword lower_bound =  align_up(address, PAGE_SIZE);
 
                // Deallocating pages
                deallocate_memory(lower_bound, current_break_page_num);

                r->eax = Ec::break_current;
                Ec::break_current = address;
                break;
            }
        }
        default:
            printf ("unknown syscall %d\n", number);
            break;
    };
 
    ret_user_sysexit();
}