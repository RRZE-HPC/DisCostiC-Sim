// for (int level = 1; level < numberOfMgLevels; ++level) {
// GenerateCoarseProblem(*curLevelMatrix);
// curLevelMatrix = curLevelMatrix->Ac;
// }
// for (int level = 0; level < numberOfMgLevels; ++level) {
// CheckProblem(*curLevelMatrix, curb, curx, curxexact);
// curLevelMatrix = curLevelMatrix->Ac;
// curb = 0;
// curx = 0;
// curxexact = 0;
// }
// for (int i = 0; i < numberOfCalls; ++i) {
// ierr = ComputeSPMV_ref(A, x_overlap, b_computed);
// ierr = ComputeMG_ref(A, b_computed, x_overlap);
// }
for (int i = 0; i < numberOfCalls; ++i) {
ZeroVector(x);
ierr = CG_ref(A, data, b, x, refMaxIters, tolerance, niters, normr, normr0, &ref_times[0], true);
if (ierr) ++err_count;
totalNiters_ref += niters;
}
// for (int i = 0; i < numberOfCalls; ++i) {
// ZeroVector(x);
// double last_cummulative_time = opt_times[0];
// ierr = CG(A, data, b, x, optMaxIters, refTolerance, niters, normr, normr0, &opt_times[0], true);
// if (ierr) ++err_count;
// if (normr / normr0 > refTolerance) ++tolerance_failures;
// if (niters > optNiters) optNiters = niters;
// double current_time = opt_times[0] - last_cummulative_time;
// if (current_time > opt_worst_time) opt_worst_time = current_time;
// }
// for (int i = 0; i < numberOfCgSets; ++i) {
// ZeroVector(x);
// ierr = CG(A, data, b, x, optMaxIters, optTolerance, niters, normr, normr0, &times[0], true);
// testnorms_data.values[i] = normr / normr0;
// }

