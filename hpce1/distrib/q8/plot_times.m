% Plot graphs

powerLower = 2;
powerUpper = 11;
w = round(2.^(powerLower:0.5:powerUpper));
h = w;
xLimit = [2^(powerLower*2) 2^(powerUpper*2)];

% Comparison of scalar vs vector vs compiled in non parallerised
[tOriginal, pixels] = time_renderer(rjv2(@julia_v1), w, h);
tVector = time_renderer(rjv2(@julia_v2), w, h);
tCompile = time_renderer(rjv2(@julia_v4_mex), w, h);

figure(1);
loglog(pixels, tOriginal, pixels, tVector, pixels, tCompile);
xlim(xLimit);
xlabel('Frame Pixel Count');
ylabel('Running Time/s');
legend('Scalar', 'Vectorised', 'Compiled Scalar', 4);
title('Comparison of `julia` running sequentially');
grid on;

print -dpdf 'sequential-comparison.pdf';

% Comparison of scalar vs vector compiled parallerised
matlabpool close;
matlabpool 4;
figure(2);
tpOriginal = time_renderer(rjv2(@julia_v1, @render_julia_v3), w, h);
tpVector = time_renderer(rjv2(@julia_v2, @render_julia_v3), w, h);
tpCompile = time_renderer(rjv2(@julia_v4_mex, @render_julia_v3), w, h);

loglog(pixels, tpOriginal, pixels, tpVector, pixels, tpCompile);
xlim(xLimit);
xlabel('Frame Pixel Count');
ylabel('Running Time/s');
legend('Scalar','Vectorised', 'Compiled Scalar', 4);
title('Comparison of `julia` running in parallel');
grid on;

print -dpdf 'parallel-comparison.pdf';

% Comparison of sequential vs parallerised
figure(3);

loglog(pixels, tOriginal, pixels, tpOriginal,...
    pixels, tVector, pixels, tpVector,...
    pixels, tCompile, pixels, tpCompile);
xlim(xLimit);
xlabel('Frame Pixel Count');
ylabel('Running Time/s');
legend('Scalar Sequential', 'Scalar Parallerised',...
    'Vectorised Sequential', 'Vectorised Parallerised',...
    'Compiled Sequential', 'Compiled Parallerised',...
    4);
title('Comparison of computation run sequentially vs parallerised');
grid on;

print -dpdf 'sequential-vs-parallerised.pdf';

% Comparison of number of workers
figure(4);

matlabpool close;
matlabpool 2;
tWorker2 = time_renderer(rjv2(@julia_v1, @render_julia_v3), w, h);

matlabpool close;
matlabpool 3;
tWorker3 = time_renderer(rjv2(@julia_v1, @render_julia_v3), w, h);


loglog(pixels, tOriginal, pixels, tWorker2, pixels, tWorker3,...
    pixels, tpOriginal);
xlim(xLimit);
xlabel('Frame Pixel Count');
ylabel('Running Time/s');
legend('Sequential', '2 Workers', '3 Workers', '4 Workers', 4);
title('Comparison of scalar `julia` running with different number of workers');
grid on;

print -dpdf 'worker-count-comparison.pdf';