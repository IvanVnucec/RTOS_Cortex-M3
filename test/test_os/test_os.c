#include "minunit.h"


void test_setup(void) {

}


void test_teardown(void) {
	/* Nothing */
}


MU_TEST(test_check) {
	mu_check(1 == 2);
}


MU_TEST_SUITE(test_suite) {
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

	MU_RUN_TEST(test_check);
}

int main(void) {
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
    
	return MU_EXIT_CODE;
}
