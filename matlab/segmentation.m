function frag_depthmap=segmentation(depthmap,halfPatchSize,reconstruct,k)
    frag_depthmap=depthmap;
    dim=size(depthmap);

    
    distList=1000000000*ones(1,k);
    depthList=10*ones(1,k);
    
    for px=1+halfPatchSize:dim(1)-halfPatchSize
        for py=1+halfPatchSize:dim(2)-halfPatchSize
            
            for(tx=px-halfPatchSize:4:px+halfPatchSize)
                for(ty=py-halfPatchSize:4:py+halfPatchSize)
                   dist=sum(power(reconstruct(tx,ty)-reconstruct(px,py),2));
                   if dist<distList(k)
                       distList(k)=dist;
                       depthList(k)=depthmap(tx,ty);
                       [distList,order]=sort(distList);
                       depthList=depthList(order);
                   end
                end
            end
            frag_depthmap(px,py)=mode(depthList);
        end
    end
end

