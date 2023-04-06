/*
 * Do a electrical test of the tester itself
 */
int tests_selfTest(void);
int tests_setupBoard(struct config const *b_cfg);
int tests_checkVoltages(struct config const *b_cfg);
int tests_checkPullDown(struct config const *b_cfg);
int tests_checkLogic(struct config const *b_cfg, char *vector);
