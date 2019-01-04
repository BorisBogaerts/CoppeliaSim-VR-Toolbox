/*=========================================================================
This shader code will be injected in the standard VTK polydataMapper shader
template. It has to be valid shader code

If you want to customize best contact: boris.bogaerts@uantwerpen.be

Input :
	vec2 tcoordVCVSOutput texture coordinates (used to determine pixel location)
	
Output :
	vec4 fragOutput0 Pixel color, RGB and Opacity
=========================================================================*/

if (tcoordVCVSOutput[1]>0.5){
	fragOutput0[0] = 0.5;
	fragOutput0[1] = 0;
	fragOutput0[2] = 0;
	fragOutput0[3] = 1;
}else{
	fragOutput0[0] = 0;
	fragOutput0[1] = 0.5;
	fragOutput0[2] = 0;
	fragOutput0[3] = 1;
};
