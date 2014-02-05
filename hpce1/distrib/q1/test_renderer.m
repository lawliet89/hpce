function [] = test_renderer(renderer)
    % test_renderer checks that a renderer is functionally correct,
    % by checking it against the reference render_julia function
    % for various (w,h,c,maxiter) test?points.
    % The set of test?points chosen is up to the implementation,
    % and should be chosen to balance likelihood of error?detection
    % against execution time.
    % The function should fail using assert if any test fails, or
    % run to completion otherwise.
    
    % Test parameters
    w = round(2.^(4:0.5:10));
    h = w;
    c = sqrt(-2);
    maxiter = 16;

    for i = 1:size(w,2)
       expected = render_julia(w(1, i), h(1, i), c, maxiter);
       actual = renderer(w(1, i), h(1, i), c, maxiter);
       
       assert(all(all(expected == actual)));
    end
    disp('All Pass');
end