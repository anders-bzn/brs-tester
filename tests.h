/*
 * BRS-tester by Anders Sandahl 2023-2024
 *
 * License GPL 2.0
 *
 */

/*
 * Do a electrical test of the tester itself
 */
int tests_selfTest(void);
int tests_setupBoard(struct config const *b_cfg);
int tests_checkVoltages(struct config const *b_cfg);
int tests_checkPullDown(struct config const *b_cfg);
int tests_checkLogic(struct config const *b_cfg, char *vector, bool singleStep);
int tests_checkDriveStrength(struct config const *b_cfg, char *vector, bool singleStep);
int tests_checkInputs(struct config const *b_cfg);
