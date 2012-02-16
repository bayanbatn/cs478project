// default.frag 

#extension GL_OES_EGL_image_external:enable

precision mediump float;

varying mediump vec2 vTexCoordOut;
uniform samplerExternalOES texture;

void main() {
	vec4 color = texture2D(texture, vTexCoordOut);
	if (color[0] > 0.99 && color[1] > 0.99 && color[2] > 0.99)
	{
		float fx = vTexCoordOut[0]*800.0;
		float fy = vTexCoordOut[1]*600.0;
		vec2 fTexCoord = vec2(fx, fy);
		ivec2 intTexCoord = ivec2(fTexCoord);
		int temp = (intTexCoord[0] + intTexCoord[1]) / 10;
		if (((temp + 1) / 2) - (temp / 2) == 0) //most awkward way to do modulo op
			gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		else 
			gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	}	
	else
		gl_FragColor = color;
}
