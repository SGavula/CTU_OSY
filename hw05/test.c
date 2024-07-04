
while(1) {
    mutex_lock
    while(condition) {
        if(condition to exit is met) return null (exit thread)
        waiting thread if condition is not met
        pthread_wait_cond()
    }

    executing the thread if condition is met
    broadcast
    mutex_unlock
}