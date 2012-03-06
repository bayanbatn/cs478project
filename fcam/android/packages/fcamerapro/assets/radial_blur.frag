// spherize.frag

/*
  This filer applies a "spherize" effect,
  which distorts the image at a particular
  position and radius.
*/

// Get the image-space position generated in the vertex program.
varying vec2 pos;

// The input image we will be filtering in this kernel.
uniform sampler2D image;

//
uniform vec2 pixelSize;
uniform vec2 imageSize;

//
uniform vec2 center;
uniform float amount;

void main()
{

	// get original color
	vec4 color = texture2D(image, pos);

	// direction from the point pointing into the center
	vec2 dir = center - pos;
	float distance = length(dir);
	// normalize direction vector
	dir = dir / distance; 

	// sum up some sample pixels around
	// starting with the color at the position
	vec4 blurred = color;

	for (int i = 0; i< 40; i++){
		blurred += texture2D(image, pos + (0.5* distance)*(5*amount)*dir* 0.01*i);
	}
	blurred /= 41.0; // normalized 

	// adjust to original brightness
	blurred = blurred / length(blurred) * length(color);

    gl_FragColor = blurred;
}
