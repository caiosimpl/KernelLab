#include "tests.h"

void app_main(void) {
    test_table();
	test_context_switch();
    test_round_robin();
    test_round_robin_queue_full();
}
