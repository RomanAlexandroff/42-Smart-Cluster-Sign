/*
 * ============================================================
 *  42 Smart Cluster Sign Unit Test (Check framework)
 * ============================================================
 *
 *  Compilation on macOS:
 *      cc test_42-Smart-Cluster-Sign.c -lcheck -lpthread -lm
 * *  Compilation on Linux:
 *      cc test_42-Smart-Cluster-Sign.c -lcheck -lpthread -lm -lsubunit
 *
 * ============================================================
 */

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
//#include "../../src/42-Smart-Cluster-Sign.h"

/* ============================================================
 *  TESTS
 * ============================================================
 */

/* ============================================================
   MOCKS
   ============================================================ */

static int mock_adc_value;
static int wifi_status_value;
static int wifi_connect_called;
static int sleep_called;
static int message_sent;
static int display_called;

#define BATTERY_SAMPLES_LIMIT 	5
#define BATTERY_GOOD 			750
#define BATTERY_CRITICAL 		400
#define WL_CONNECTED 			1
#define LOW_BATTERY 			1005
#define DEAD_BATTERY 			1006
#define DEAD_BATTERY_SLEEP_MS 	86400000ull

/* ---- Mocked external functions ---- */

int adc1_get_raw(int channel)
{
    (void)channel;
    return mock_adc_value;
}

void delay(int ms)
{
	(void)ms;
}

void watchdog_reset(void) {}

int WiFi_status(void)
{
    return (wifi_status_value);
}

void wifi_connect(void)
{
    wifi_connect_called = 1;
}

void display_cluster_number(int x)
{
    (void)x;
    display_called = 1;
}

void go_to_sleep(int ms)
{
    (void)ms;
    sleep_called = 1;
}

void bot_sendMessage(void)
{
    message_sent = 1;
}

/* ============================================================
   Include production file AFTER mocks
   ============================================================ */

#include "../../src/battery_management.cpp"

/* ============================================================
   TESTS
   ============================================================ */

START_TEST(test_battery_good_does_nothing)
{
    mock_adc_value = 950;
    wifi_status_value = WL_CONNECTED;

    battery_check();

    ck_assert_int_eq(display_called, 0);
    ck_assert_int_eq(message_sent, 0);
    ck_assert_int_eq(sleep_called, 0);
}
END_TEST


START_TEST(test_battery_low_sends_warning)
{
    mock_adc_value = 500;
    wifi_status_value = WL_CONNECTED;

    battery_check();

    ck_assert_int_eq(display_called, 1);
    ck_assert_int_eq(message_sent, 1);
    ck_assert_int_eq(sleep_called, 0);
}
END_TEST


START_TEST(test_battery_critical_goes_to_sleep)
{
    mock_adc_value = 390;
    wifi_status_value = WL_CONNECTED;

    battery_check();

    ck_assert_int_eq(display_called, 1);
    ck_assert_int_eq(message_sent, 1);
    ck_assert_int_eq(sleep_called, 1);
}
END_TEST


/* ============================================================
 *  TEST SUITE CREATION
 * ============================================================
 */
static Suite *create_test_suite(void)
{
    Suite  *suite;
    TCase  *core_case;

    suite = suite_create("Environment");
    core_case = tcase_create("Core");

    tcase_add_test(core_case, test_battery_good_does_nothing);
    tcase_add_test(core_case, test_battery_low_sends_warning);
    tcase_add_test(core_case, test_battery_critical_goes_to_sleep);

    suite_add_tcase(suite, core_case);

    return suite;
}


/* ============================================================
 *  MAIN
 * ============================================================
 */
int main(void)
{
    Suite   *suite = create_test_suite();
    SRunner *runner = srunner_create(suite);

    srunner_run_all(runner, CK_VERBOSE);

    int failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}