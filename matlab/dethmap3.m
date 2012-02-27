function dethmap3

% depth map patch is 2*halfPatchSize by 2*halfPatchSize
halfPatchSize=20;
Im1=imread('t5.jpg','JPG');
Im2=imread('t4.jpg','JPG');
Im3=imread('t3.jpg','JPG');
Im4=imread('t2.jpg','JPG');
Im5=imread('t1.jpg','JPG');

%unit : cm
focal=[500,100,40,20,10];   

%grayscale image
g1 = rgb2gray(Im1);
g2 = rgb2gray(Im2);
g3 = rgb2gray(Im3);
g4 = rgb2gray(Im4);
g5 = rgb2gray(Im5);

%sharpness image
sh1=sharpness(g1);
sh2=sharpness(g2);
sh3=sharpness(g3);
sh4=sharpness(g4);
sh5=sharpness(g5);

%initial depth map
dim=size(sh1);
depthmap=zeros(dim(1),dim(2));

% calculate the sharpness of image at each focus
% and choose the focus with max local sharpness
for x=1+halfPatchSize-1:2*halfPatchSize:dim(1)-halfPatchSize 
    for y=1+halfPatchSize-1:2*halfPatchSize:dim(2)-halfPatchSize 
        
        f(1)=sum(sum(sh1(x-halfPatchSize+1:x+halfPatchSize,y-halfPatchSize+1:y+halfPatchSize)));
        f(2)=sum(sum(sh2(x-halfPatchSize+1:x+halfPatchSize,y-halfPatchSize+1:y+halfPatchSize)));
        f(3)=sum(sum(sh3(x-halfPatchSize+1:x+halfPatchSize,y-halfPatchSize+1:y+halfPatchSize)));
        f(4)=sum(sum(sh4(x-halfPatchSize+1:x+halfPatchSize,y-halfPatchSize+1:y+halfPatchSize)));
        f(5)=sum(sum(sh5(x-halfPatchSize+1:x+halfPatchSize,y-halfPatchSize+1:y+halfPatchSize)));
 
        [c,i]=max(f);
        depthmap(x-halfPatchSize+1:x+halfPatchSize,y-halfPatchSize+1:y+halfPatchSize)=i*ones(2*halfPatchSize,2*halfPatchSize);
        depthmap_focal(x-halfPatchSize+1:x+halfPatchSize,y-halfPatchSize+1:y+halfPatchSize)=focal(i)*ones(2*halfPatchSize,2*halfPatchSize);
    end
end
max_depthmap=max(max(depthmap));
a=imshow(depthmap,[min(depthmap(:)) max(depthmap(:))]);
saveas(a,'depthmap.jpeg');


%% the original picture
reconstruct=imread('t1.jpg','JPG');
for x=1:dim(1)
    for y=1:dim(2) 
        if depthmap(x,y)==1
            reconstruct(x,y,:)=Im1(x,y,:);
        elseif depthmap(x,y)==2
            reconstruct(x,y,:)=Im2(x,y,:);
        elseif depthmap(x,y)==3
            reconstruct(x,y,:)=Im3(x,y,:);
        elseif depthmap(x,y)==4
            reconstruct(x,y,:)=Im4(x,y,:);
        elseif depthmap(x,y)==5
            reconstruct(x,y,:)=Im5(x,y,:);
        end                
    end
end
imshow(reconstruct);
imwrite(reconstruct,'reconstruct.png','png');




%{
%% segmentation
k=5;
frag_depthmap=segmentation(depthmap,halfPatchSize,reconstruct,k);
a=imshow(frag_depthmap,[min(frag_depthmap(:)) max(frag_depthmap(:))]);


%%
ref=im2double(g1);
for i=1:9
filtered_depthmap = bfltGray(c_dmap,ref,20,0.5,0.5)*max_depthmap;
reconstruct=imread('paper_focus1.png','PNG');

a=imshow(filtered_depthmap,[min(filtered_depthmap(:)) max(filtered_depthmap(:))]);

eval(['   saveas(a,''depthmap',int2str(i),'.jpeg'');     ']);


end







for x=1:s(1) 
    for y=1:s(2) 
        if filtered_depthmap(x,y)==1
            reconstruct(x,y,:)=Im1(x,y,:);
        elseif depthmap(x,y)==2
            filtered_depthmap(x,y,:)=Im2(x,y,:);
        elseif depthmap(x,y)==3
            filtered_depthmap(x,y,:)=Im3(x,y,:);
        elseif depthmap(x,y)==4
            filtered_depthmap(x,y,:)=Im4(x,y,:);
        elseif depthmap(x,y)==5
            filtered_depthmap(x,y,:)=Im5(x,y,:);
        end                
            
            
    end
end
eval(['imwrite(reconstruct,''reconstruct',int2str(i),'.png'',''png'');']);
imshow(reconstruct);



%}

