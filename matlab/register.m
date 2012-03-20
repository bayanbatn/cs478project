

base = imread('img_0000_09.jpg');
tforms = [];
input_points = [];
base_points = [];

for i=0:8

    input = imread(['img_0000_0' num2str(i) '.jpg']);

    % Block the tool until you pick some more control points
    if (i == 0) 
        [input_points, base_points] = ...
               cpselect(input,base,...
                        'Wait',true);
    else 
        [input_points, base_points] = ...
               cpselect(input,base,...
                        input_points,base_points,...
                        'Wait',true);
    end 
    
    tform = cp2tform(input_points,base_points,'nonreflective similarity');
    registered = imtransform(input, tform,...
                                    'XData',[1 640],...
                                    'YData',[1 480]);
                                
    figure, imshow(registered)
    
    tforms = [tforms; tform];

end