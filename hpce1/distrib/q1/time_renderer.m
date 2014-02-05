function [t,n,w,h,c,maxiter] = time_renderer(renderer,w,h,c,maxiter)
    % time_renderer measures execution time for varying input parameters
    % renderer : handle to a function with signature ...
    % renderer(w,h,maxiter)
    % w,h,c,maxiter : Points at which to measure execution time,
    % Each can be a vector or a scalar, but if there ...
    % are any
    % vectors, then all vectors must have the same ...
    % length.
    % If no args are specified, the defaults should be
    % w=round(2.^(4:0.5:10)); h=w; c=sqrt(2); maxiter=64;
    % Returns:
    % t : execution time per frame
    % n : Total pixels for each frame
    % w,h,c,maxiter : Parameters corresponding to each frame.
    
    length = 1;
    if nargin < 2
        w = round(2.^(4:0.5:10));       
    end
    if (~isscalar(w))
        length = checkLength(length, size(w, 2));
    end
    
    if nargin < 3
        h = round(2.^(4:0.5:10));
    end
    if (~isscalar(h))
        length = checkLength(length, size(h, 2));
    end
    
    if nargin < 4
        c = sqrt(2);
    end
    if (~isscalar(c))
        length = checkLength(length, size(c, 2));
    end
    
    if nargin < 5
        maxiter = 64;
    end
    if (~isscalar(maxiter))
        length = checkLength(length, size(maxiter, 2));
    end
    
    t = zeros(1, length);
    n = zeros(1, length);
    for i = 1:length
        if (isscalar(w))
            width = w;
        else
            width = w(1, i);
        end
        
        if (isscalar(h))
            height = h;
        else
            height = h(1, i);
        end
        
        if (isscalar(c))
            constant = c;
        else
            constant = c(1, i);
        end
        
        if (isscalar(maxiter))
            iter = maxiter;
        else
            iter = maxiter(1, i);
        end        
        
        tic;
        renderer(width, height, constant, iter);
        t(1, i) = toc;
        
        n(1, i) = width * height;
    end
end

function length = checkLength(length, new)
    if length == 1
       length = new; 
    else
        assert(length == new);
    end
end