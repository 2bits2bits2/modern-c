#include "mctest.h"
#include "tournament.h"

/*
 * A whole tournament, simulated instantly. No sleeping, no flakiness: we own
 * the clock, so an hour of play takes microseconds.
 */
static void test_a_whole_tournament(void)
{
    struct tournament tournament;

    tournament_start(&tournament, 3, 1); /* 3 levels, 1 minute each */

    tournament_advance(&tournament, 30 * 1000);
    CHECK_INT(tournament_alerts(&tournament), 0);

    tournament_advance(&tournament, 30 * 1000); /* 1 minute */
    CHECK_INT(tournament_alerts(&tournament), 1);

    tournament_advance(&tournament, 60 * 1000); /* 2 minutes */
    CHECK_INT(tournament_alerts(&tournament), 2);

    tournament_advance(&tournament, 60 * 1000); /* 3 minutes */
    CHECK_INT(tournament_alerts(&tournament), 3);

    tournament_advance(&tournament, 60 * 60 * 1000); /* an hour later */
    CHECK_INT(tournament_alerts(&tournament), 3);
}

int main(void)
{
    RUN_TEST(test_a_whole_tournament);
    return test_summary();
}
