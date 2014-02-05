function [viter,vz] = julia_v1(vz, c, maxiter)
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
