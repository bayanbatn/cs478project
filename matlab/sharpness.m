function [diff] = sharpness(grayscale)
    s=size(grayscale);
    diff=zeros(s(1),s(2));
    for i=2:s(1)-1
        for j=2:s(2)-1
            
            %{
            v(1)=abs(grayscale(i-1,j)-grayscale(i+1,j));
            v(2)=abs(grayscale(i,j-1)-grayscale(i,j+1));
            v(3)=abs(grayscale(i-1,j-1)-grayscale(i+1,j+1));
            v(4)=abs(grayscale(i-1,j+1)-grayscale(i+1,j-1));
            %sort(v);
            %diff(i,j)=sum(v(1:3));
            diff(i,j)=sum(v);
            %diff(i,j)=abs(grayscale(i-1,j)-grayscale(i+1,j))+abs(grayscale(i,j-1)-grayscale(i,j+1));
            %}
            
            
            v(1)=2*abs(grayscale(i-1,j)-grayscale(i+1,j));%x
            v(2)=abs(grayscale(i-1,j-1)-grayscale(i+1,j-1));%x
            v(3)=abs(grayscale(i-1,j+1)-grayscale(i+1,j+1));%x
            
            v(4)=2*abs(grayscale(i,j-1)-grayscale(i,j+1));%y
            v(5)=abs(grayscale(i-1,j-1)-grayscale(i-1,j+1));%y
            v(6)=abs(grayscale(i+1,j-1)-grayscale(i+1,j+1));%y
            
            diff(i,j)=power(power(sum(v(1:3)),2)+power(sum(v(4:6)),2),0.5);
            
            
        end
    end
end