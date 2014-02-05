function [viter,vz] = julia_v4(vz, c, maxiter)
    
    % for code generation poruposes
    %vz
    assert(isa(vz,'double'));
    assert(~isreal(vz));
    assert(all(size(vz)<=[1 Inf]));
    % c
    assert(isa(c,'double'));
    assert(~isreal(c));
    assert(isscalar(c));
    % maxiter 
    assert(isa(maxiter,'int32'));
    assert(isscalar(maxiter));
    
    viter = ones(1, size(vz, 2));
    for i = 1:size(vz, 2)
        while viter(1, i) < maxiter
            if abs(vz(1, i)) > 1
                break
            end;
            viter(1, i) = viter(1, i) + 1;
            vz(1, i) = vz(1, i).^2 + c;
        end
    end
end
