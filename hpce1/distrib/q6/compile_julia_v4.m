% Generate MEX file for Julia
mex_cfg = coder.config('mex');
mex_cfg.IntegrityChecks = false;
mex_cfg.ResponsivenessChecks = false;
mex_cfg.GenerateReport = true;

codegen -config mex_cfg julia_v4