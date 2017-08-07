// ONLY GENERIC STATIC FUNCTIONS
// FOR OBJECTS IN */js/class/main.tpl

<!-- temporary close js -->
</script>

<script id="wood-vs" type="x-shader/x-vertex">
    precision mediump   float;

    attribute vec3      aPosition;
    // attribute vec3      aNormal;
    attribute vec2      aTexCoord;
    attribute float     aFaceIndex;
    attribute vec4      aSpot;

    uniform mat4        uPMatrix;
    uniform mat4        uMVMatrix;
    uniform mat4        uZMatrix;
    uniform float       uScale;
    uniform vec3        uOffset;
    uniform float       uSelected;

    varying vec2        vSurfTex;
    varying vec3        vVolTex;
    varying vec4        vHighlight;
    varying vec4        vSpot;

    void main(void){
        gl_Position = uPMatrix
            * uZMatrix
            * uMVMatrix
            * vec4(aPosition, 1.0);

        vSurfTex    = aTexCoord;
        vVolTex     = aPosition * uScale + uOffset;

        vHighlight  = max(
            1.0 - abs(uSelected), // - aFaceIndex),
            0.0
        ) * vec4(
            0.5,
            0.5,
            0.5,
            0.0
        );

        vSpot = aSpot;
    }
</script>

<script id="wood-fs" type="x-shader/x-fragment">
    precision mediump float;

    uniform float   uScale;
    uniform vec3    uOffset;

    varying vec2    vSurfTex;
    varying vec3    vVolTex;
    varying vec4    vHighlight;
    varying vec4    vSpot;

    //
    // Description : Array and textureless GLSL 2D/3D/4D simplex
    // noise functions.
    // Author : Ian McEwan, Ashima Arts.
    // Maintainer : ijm
    // Lastmod : 20110822 (ijm)
    // License : Copyright (C) 2011 Ashima Arts. All rights reserved.
    // Distributed under the MIT License. See LICENSE file.
    // https://github.com/ashima/webgl-noise
    //

    vec3 mod289(vec3 x){
        return x - floor(x * (1.0 / 289.0)) * 289.0;
    }

    vec4 mod289(vec4 x){
        return x - floor(x * (1.0 / 289.0)) * 289.0;
    }

    vec4 permute(vec4 x){
        return mod289(((x*34.0)+1.0)*x);
    }

    vec4 taylorInvSqrt(vec4 r){
        return 1.79284291400159 - 0.85373472095314 * r;
    }

    float snoise(vec3 v){
        float       ret = 0.0;
        const vec2  C   = vec2(1.0/6.0, 1.0/3.0);
        const vec4  D   = vec4(0.0, 0.5, 1.0, 2.0);

        // First corner
        vec3    i   = floor(v + dot(v, C.yyy) );
        vec3    x0  = v - i + dot(i, C.xxx) ;

        // Other corners
        vec3    g   = step(x0.yzx, x0.xyz);
        vec3    l   = 1.0 - g;
        vec3    i1  = min( g.xyz, l.zxy );
        vec3    i2  = max( g.xyz, l.zxy );

        // x0 = x0 - 0.0 + 0.0 * C.xxx;
        // x1 = x0 - i1 + 1.0 * C.xxx;
        // x2 = x0 - i2 + 2.0 * C.xxx;
        // x3 = x0 - 1.0 + 3.0 * C.xxx;
        vec3    x1  = x0 - i1 + C.xxx;
        vec3    x2  = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
        vec3    x3  = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

        // Permutations
        i = mod289(i);
        vec4 p = permute(
            permute(
                permute(
                     i.z + vec4(0.0, i1.z, i2.z, 1.0)
                )
                + i.y + vec4(0.0, i1.y, i2.y, 1.0 )
            )
            + i.x + vec4(0.0, i1.x, i2.x, 1.0 )
        );

        // Gradients: 7x7 points over a square,
        // mapped onto an octahedron.
        // The ring size 17*17 = 289
        // is close to a multiple of 49 (49*6 = 294)
        float   n_  = 0.142857142857; // 1.0/7.0
        vec3    ns  = n_ * D.wyz - D.xzx;

        vec4    j   = p - 49.0 * floor(p * ns.z * ns.z); // mod(p,7*7)

        vec4    x_  = floor(j * ns.z);
        vec4    y_  = floor(j - 7.0 * x_ ); // mod(j,N)

        vec4    x   = x_ *ns.x + ns.yyyy;
        vec4    y   = y_ *ns.x + ns.yyyy;
        vec4    h   = 1.0 - abs(x) - abs(y);

        vec4    b0  = vec4( x.xy, y.xy );
        vec4    b1  = vec4( x.zw, y.zw );

        //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
        //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
        vec4    s0  = floor(b0)*2.0 + 1.0;
        vec4    s1  = floor(b1)*2.0 + 1.0;
        vec4    sh  = -step(h, vec4(0.0));

        vec4    a0  = b0.xzyw + s0.xzyw*sh.xxyy ;
        vec4    a1  = b1.xzyw + s1.xzyw*sh.zzww ;

        vec3    p0  = vec3(a0.xy, h.x);
        vec3    p1  = vec3(a0.zw, h.y);
        vec3    p2  = vec3(a1.xy, h.z);
        vec3    p3  = vec3(a1.zw, h.w);

        // Normalise gradients
        vec4    norm = taylorInvSqrt(
            vec4(
                dot(p0, p0),
                dot(p1, p1),
                dot(p2, p2),
                dot(p3, p3)
            )
        );
        p0 *= norm.x;
        p1 *= norm.y;
        p2 *= norm.z;
        p3 *= norm.w;

        // Mix final noise value
        vec4 m = max(
            0.6 - vec4(
                dot(x0, x0),
                dot(x1, x1),
                dot(x2, x2),
                dot(x3, x3)
            ),
            0.0
        );
        m   = m * m;
        ret = 42.0 * dot(
            m*m,
            vec4(
                dot(p0,x0),
                dot(p1,x1),
                dot(p2,x2),
                dot(p3,x3)
            )
        );
        return ret;
    }

    // This code does not fall
    // under the above copyright statement
    void main(void){
        // larger -> more rings
        float ringFrequency     = 12.0;

        // larger -> thicker rings
        float ringScale         = 1.0;

        // larger -> more distortion
        float noiseMagnitude    = 0.7;

        // larger -> finer details
        float noiseScale        = 2.0;

        vec4 darkWood  = vec4(0.44, 0.21, 0.00, 1.0);
        vec4 lightWood = vec4(0.91, 0.50, 0.13, 1.0);

        float noise = snoise(vVolTex*noiseScale)
            + snoise(vVolTex*noiseScale*2.0 ) / 2.0
            + snoise(vVolTex*noiseScale*8.0 ) / 4.0;
            // + snoise(vVolTex*noiseScale*16.0) / 8.0;

        float ring = fract(
            ringFrequency * length(vVolTex.xy)
            + noiseMagnitude * noise
        );
        ring *= 4.0 * (1.0 - ring);

        float lrp = pow(ring, ringScale) + noise*0.5;

        gl_FragColor = mix(darkWood, lightWood, lrp)
            + vHighlight;

        if (length(vSurfTex - vec2(0.5, 0.5)) < 0.1){
            gl_FragColor = mix(
                gl_FragColor,
                vec4(vSpot.rgb, 1.0),
                vSpot.a
            );
        }
    }
</script>

<!-- back to js -->
<script type="text/javascript">

