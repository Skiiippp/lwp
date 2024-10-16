#include <lwp.h>
#include <stdio.h>

int test_lwp_function(void *arg);

int main() {
    lwp_create(test_lwp_function, (void *)("Hello"));
    lwp_create(test_lwp_function, (void *)("World"));

    lwp_start();

    while (lwp_wait(NULL) != NO_THREAD)
        ;

    printf("yo we here!\n");

    return 0;
}

int test_lwp_function(void *arg) {
    printf("%s\n", (char *)arg);
    return 0;
}