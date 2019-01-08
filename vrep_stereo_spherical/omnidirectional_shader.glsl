/*=========================================================================
This shader code will be injected in the standard VTK polydataMapper shader
template. It has to be valid shader code

If you want to customize best contact: boris.bogaerts@uantwerpen.be

Input :
	vec2 tcoordVCVSOutput texture coordinates (used to determine pixel location)
	
Output :
	vec4 fragOutput0 Pixel color, RGB and Opacity
=========================================================================*/

float u,v, x, y, z;
bool a, b, c;
u = - 3.1415 + ( 2 * 3.1415 * tcoordVCVSOutput[0]);

if (tcoordVCVSOutput[1]>0.5){												// Left - stereo
	//v = (3.1415/2) - ( 2 * 3.1415 *(tcoordVCVSOutput[1]-0.5));
	
	//x = sin(u)*cos(v);
	//y = sin(v);
	//z = -cos(u)*cos(v);
	
	
	if((tcoordVCVSOutput[0]<=(1/8.0)) || (tcoordVCVSOutput[0]>(7/8.0)) ){ 		// Back viewµ
		if (tcoordVCVSOutput[1]>0.75){												// Top cam
			fragOutput0[0] = 0.5;
			fragOutput0[1] = 0;
			fragOutput0[2] = 0;
			fragOutput0[3] = 1;
		}else{																		// Bottom cam
			fragOutput0[0] = 1;
			fragOutput0[1] = 0;
			fragOutput0[2] = 0;
			fragOutput0[3] = 1;
		}
		

	}else if((tcoordVCVSOutput[0]<=(3/8.0))){ 									// right view
		if (tcoordVCVSOutput[1]>0.75){												// Top cam
			fragOutput0[0] = 0.5;
			fragOutput0[1] = 0.5;
			fragOutput0[2] = 0.5;
			fragOutput0[3] = 1;
		}else{																		// Bottom cam
			fragOutput0[0] = 0;
			fragOutput0[1] = 0;
			fragOutput0[2] = 1;
			fragOutput0[3] = 1;
		}
		
	
	}else if((tcoordVCVSOutput[0]<=(5/8.0))){ 									// front view
		if (tcoordVCVSOutput[1]>0.75){												// Top cam
			fragOutput0[0] = 0.5;
			fragOutput0[1] = 0;
			fragOutput0[2] = 0.5;
			fragOutput0[3] = 1;
		}else{																		// Bottom cam
			fragOutput0[0] = 1;
			fragOutput0[1] = 0;
			fragOutput0[2] = 1;
			fragOutput0[3] = 1;
		}
		
	
	}else if((tcoordVCVSOutput[0]<=(7/8.0))){ 									// left view
		if (tcoordVCVSOutput[1]>0.75){												// Top cam
			fragOutput0[0] = 0.5;
			fragOutput0[1] = 0.5;
			fragOutput0[2] = 0;
			fragOutput0[3] = 1;
		}else{																		// Bottom cam
			fragOutput0[0] = 1;
			fragOutput0[1] = 1;
			fragOutput0[2] = 0;
			fragOutput0[3] = 1;
		}
		
	}
	

	
}else{																		// Right - stereo
	fragOutput0[0] = 0;
	fragOutput0[1] = 0.5;
	fragOutput0[2] = 0;
	fragOutput0[3] = 1;
};
