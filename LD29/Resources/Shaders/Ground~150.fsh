//
//  rn_Default.fsh
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

#include <shader/rn_Lighting.fsh>


// internal define setup
#if defined(RN_LIGHTING)
	#define INTERNAL_NEEDS_POSITION_WORLD 1
	#define INTERNAL_NEEDS_NORMAL_WORLD 1
#endif

#if defined(RN_FOG)
	#define INTERNAL_NEEDS_POSITION_WORLD 1
	#define INTERNAL_NEEDS_FOG 1
#endif
#if defined(RN_CLIPPLANE)
	#define INTERNAL_NEEDS_POSITION_WORLD 1
	#define INTERNAL_NEEDS_CLIPPLANE 1
#endif


//in out and uniforms

uniform vec4 diffuse;
#if defined(RN_TEXTURE_DIFFUSE)
	uniform sampler2D mTexture0;
	uniform sampler2D mTexture1;
	uniform sampler2D mTexture2;
	uniform sampler2D mTexture3;
#endif

#if defined(INTERNAL_NEEDS_FOG)
	uniform vec2 fogPlanes;
	uniform vec4 fogColor;
#endif

#if defined(INTERNAL_NEEDS_CLIPPLANE)
	uniform vec4 clipPlane;
#endif

#if defined(RN_TEXTURE_DIFFUSE)
	in vec2 vertTexcoord0;
	in vec2 vertTexcoord1;
#endif

#if defined(INTERNAL_NEEDS_POSITION_WORLD)
	in vec3 vertPosition;
#endif
#if defined(INTERNAL_NEEDS_NORMAL_WORLD)
	in vec3 vertNormal;
#endif

out vec4 fragColor0;

void main()
{
	#if defined(RN_CLIPPLANE)
		if(dot(clipPlane.xyz, vertPosition)-clipPlane.w < 0.0)
			discard;
	#endif
	
	vec4 color0 = vec4(1.0);

	#if defined(RN_TEXTURE_DIFFUSE)
		float mask = texture(mTexture0, vertTexcoord0).r;
		vec4 dirty = texture(mTexture1, vertTexcoord1);
		vec4 clean = texture(mTexture2, vertTexcoord1);
		vec4 water = texture(mTexture3, vertTexcoord1);

		color0 =  mix(clean, dirty, mask);

		#if defined(INTERNAL_NEEDS_POSITION_WORLD)
			color0 = mix(color0, water, max(min(-vertPosition.y/5.0, 1.0), 0.0));
		#endif
	#endif

	color0 *= diffuse;

	#if defined(RN_LIGHTING)
		vec4 spec = vec4(0.0f);
		vec3 normal = normalize(vertNormal);
		
		rn_Lighting(color0, spec, normal, vertPosition);
	#endif
	
	#if defined(RN_FOG)
		float camdist = max(min((length(vertPosition-viewPosition)-fogPlanes.x)*fogPlanes.y, 1.0), 0.0);
		color0 = mix(color0, fogColor, camdist);
	#endif
	
	fragColor0 = color0;
}
