function [iter,z] = julia(z, c, maxiter)

iter=1;
while iter<maxiter
    if abs(z) > 1
        break;
    end
   z=z^2 + c; 
   iter=iter+1;
end

end
