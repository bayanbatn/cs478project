// blur.frag

/*
  This kernel blurs the image a little bit.
*/

// Get the image-space position generated in the vertex program.
varying vec2 pos;

// The input image we will be filtering in this kernel.
uniform sampler2D image;

//
uniform vec2 pixelSize;

void main()
{
    vec4 sum = vec4(0.);
    float weight = 0.;

    // In order to blur the image, we will
    // average the pixels of the input image
    // at the output location and several
    // neighboring pixels.
    //
    // We can find a 'neighboring' pixel by
    // offsetting from the current position
    // by the size of a pixel.

    sum += texture2D(image, pos);
    weight += 1.;

    sum += texture2D(image, pos + pixelSize.x);
    weight += 1.;

    sum += texture2D(image, pos - pixelSize.x);
    weight += 1.;

    sum += texture2D(image, pos + pixelSize.y);
    weight += 1.;

    sum += texture2D(image, pos - pixelSize.y);
    weight += 1.;

    gl_FragColor = sum / weight;
}
