#include "polyscope/render/opengl/shaders/rules.h"

namespace polyscope {
namespace render {
namespace backend_openGL3_glfw {

// clang-format off

// possibly discards a fragment due to global rules
const ShaderReplacementRule GLSL_VERSION(
    /* rule name */ "GLSL_VERSION",
    /* replacement sources */
    {
        {"GLSL_VERSION", "#version 330 core"},
    }
);

// possibly discards a fragment due to global rules
const ShaderReplacementRule GLOBAL_FRAGMENT_FILTER(
    /* rule name */ "GLOBAL_FRAGMENT_FILTER",
    /* replacement sources */
    {
        {"GLOBAL_FRAGMENT_FILTER", "// do nothing, for now"},
    }
);


// light using a matcap texture
// input: vec3 albedoColor;
// output: vec3 litColor after lighting
const ShaderReplacementRule LIGHT_MATCAP (
    /* rule name */ "LIGHT_MATCAP",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform sampler2D t_mat_r;
          uniform sampler2D t_mat_g;
          uniform sampler2D t_mat_b;
          uniform sampler2D t_mat_k;
          vec3 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b, sampler2D t_mat_k);
        )"},
      {"GENERATE_LIT_COLOR", R"(
          vec3 litColor = lightSurfaceMat(shadeNormal, albedoColor, t_mat_r, t_mat_g, t_mat_b, t_mat_k);
      )"}
    },
    /* uniforms */ {},
    /* attributes */ {},
    /* textures */ {
      {"t_mat_r", 2},
      {"t_mat_g", 2},
      {"t_mat_b", 2},
      {"t_mat_k", 2},
    }
);

// "light" by just copying the value
// input: vec3 albedoColor;
// output: vec3 litColor after lighting
const ShaderReplacementRule LIGHT_PASSTHRU (
    /* rule name */ "LIGHT_PASSTHRU",
    { /* replacement sources */
      {"GENERATE_LIT_COLOR", "vec3 litColor = albedoColor;"}
    },
    /* uniforms */ {},
    /* attributes */ {},
    /* textures */ {}
);


// input: uniform
// output: vec3 albedoColor
const ShaderReplacementRule SHADE_BASECOLOR (
    /* rule name */ "SHADE_BASECOLOR",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform vec3 u_baseColor;
        )"},
      {"GENERATE_SHADE_COLOR", "vec3 albedoColor = u_baseColor;"}
    },
    /* uniforms */ {
      {"u_baseColor", DataType::Vector3Float},
    },
    /* attributes */ {},
    /* textures */ {}
);

// input: vec3 shadeColor
// output: vec3 albedoColor
const ShaderReplacementRule SHADE_COLOR(
    /* rule name */ "SHADE_COLOR",
    { /* replacement sources */
      {"GENERATE_SHADE_COLOR", "vec3 albedoColor = shadeColor;"}
    },
    /* uniforms */ {},
    /* attributes */ {},
    /* textures */ {}
);

// input: attribute float shadeValue
// output: vec3 albedoColor
const ShaderReplacementRule SHADE_COLORMAP_VALUE(
    /* rule name */ "SHADE_COLORMAP_VALUE",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform float u_rangeHigh;
          uniform float u_rangeLow;
          uniform sampler1D t_colormap;
        )"},
      {"GENERATE_SHADE_COLOR", R"(
          float rangeTVal = (shadeValue - u_rangeLow) / (u_rangeHigh - u_rangeLow);
          rangeTVal = clamp(rangeTVal, 0.f, 1.f);
          vec3 albedoColor = texture(t_colormap, rangeTVal).rgb;
      )"}
    },
    /* uniforms */ {
        {"u_rangeLow", DataType::Float},
        {"u_rangeHigh", DataType::Float},
    },
    /* attributes */ {},
    /* textures */ {
        {"t_colormap", 1}
    }
);

// input: attribute vec2 shadeValue2
// output: vec3 albedoColor
const ShaderReplacementRule SHADE_COLORMAP_ANGULAR2(
    /* rule name */ "SHADE_COLORMAP_ANGULAR2",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform float u_angle;
          uniform sampler1D t_colormap;
        )"},
      {"GENERATE_SHADE_COLOR", R"(
          float pi = 3.14159265359;
          float angle = atan(shadeValue2.y, shadeValue2.x) / (2. * pi) + 0.5; // in [0,1]
          float shiftedAngle = mod(angle + u_angle/(2. * pi), 1.);
          vec3 albedoColor = texture(t_colormap, shiftedAngle).rgb;
      )"}
    },
    /* uniforms */ {
        {"u_angle", DataType::Float},
    },
    /* attributes */ {},
    /* textures */ {
        {"t_colormap", 1}
    }
);

const ShaderReplacementRule SHADE_GRID_VALUE2 (
    /* rule name */ "SHADE_GRID_VALUE2",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform float u_modLen;
          uniform vec3 u_gridLineColor;
          uniform vec3 u_gridBackgroundColor;
        )"},
      {"GENERATE_SHADE_COLOR", R"(
        float mX = mod(shadeValue2.x, 2.0 * u_modLen) / u_modLen - 1.f; // in [-1, 1]
        float mY = mod(shadeValue2.y, 2.0 * u_modLen) / u_modLen - 1.f;
        float minD = min(min(abs(mX), 1.0 - abs(mX)), min(abs(mY), 1.0 - abs(mY))) * 2.; // rect distace from flipping sign in [0,1]
        float width = 0.05;
        float slopeWidthPix = 5.;
        vec2 fw = fwidth(shadeValue2);
        float scale = max(fw.x, fw.y);
        float pWidth = slopeWidthPix * scale;
        float s = smoothstep(width, width + pWidth, minD);
        vec3 albedoColor = mix(u_gridLineColor, u_gridBackgroundColor, s);
      )"}
    },
    /* uniforms */ {
       {"u_modLen", DataType::Float},
       {"u_gridLineColor", DataType::Vector3Float},
       {"u_gridBackgroundColor", DataType::Vector3Float},
    },
    /* attributes */ {},
    /* textures */ {}
);

const ShaderReplacementRule SHADE_CHECKER_VALUE2 (
    /* rule name */ "SHADE_CHECKER_VALUE2",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform float u_modLen;
          uniform vec3 u_color1;
          uniform vec3 u_color2;
        )"},
      {"GENERATE_SHADE_COLOR", R"(
        float mX = mod(shadeValue2.x, 2.0 * u_modLen) / u_modLen - 1.f; // in [-1, 1]
        float mY = mod(shadeValue2.y, 2.0 * u_modLen) / u_modLen - 1.f;
        float minD = min( min(abs(mX), 1.0 - abs(mX)), min(abs(mY), 1.0 - abs(mY))) * 2.; // rect distace from flipping sign in [0,1]
        float p = 6;
        float minDSmooth = pow(minD, 1. / p);
        // TODO do some clever screen space derivative thing to prevent aliasing
        float v = (mX * mY); // in [-1, 1], color switches at 0
        float adjV = sign(v) * minDSmooth;
        float s = smoothstep(-1.f, 1.f, adjV);
        vec3 albedoColor = mix(u_color1, u_color2, s);
      )"}
    },
    /* uniforms */ {
       {"u_modLen", DataType::Float},
       {"u_color1", DataType::Vector3Float},
       {"u_color2", DataType::Vector3Float},
    },
    /* attributes */ {},
    /* textures */ {}
);

// input vec2 shadeValue2
// output: float shadeValue
const ShaderReplacementRule SHADEVALUE_MAG_VALUE2(
    /* rule name */ "SHADEVALUE_MAG_VALUE2",
    { /* replacement sources */
      {"GENERATE_SHADE_COLOR", "float shadeValue = length(shadeValue2);"}
    },
    /* uniforms */ {},
    /* attributes */ {},
    /* textures */ {}
);


const ShaderReplacementRule ISOLINE_STRIPE_VALUECOLOR (
    /* rule name */ "ISOLINE_STRIPE_VALUECOLOR",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform float u_modLen;
          uniform float u_modDarkness;
        )"},
      {"GENERATE_SHADE_COLOR", R"(
        float modVal = mod(shadeValue, 2.0 * u_modLen);
        if(modVal > u_modLen) {
          albedoColor *= u_modDarkness;
        }
      )"}
    },
    /* uniforms */ {
        {"u_modLen", DataType::Float},
        {"u_modDarkness", DataType::Float},
    },
    /* attributes */ {},
    /* textures */ {}
);

const ShaderReplacementRule CHECKER_VALUE2COLOR (
    /* rule name */ "CHECKER_VALUE2COLOR",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform float u_modLen;
          uniform float u_modDarkness;
        )"},
      {"GENERATE_SHADE_COLOR", R"(
        vec3 albedoColorDark = albedoColor * u_modDarkness;
        float mX = mod(shadeValue2.x, 2.0 * u_modLen) / u_modLen - 1.f; // in [-1, 1]
        float mY = mod(shadeValue2.y, 2.0 * u_modLen) / u_modLen - 1.f;
        float minD = min( min(abs(mX), 1.0 - abs(mX)), min(abs(mY), 1.0 - abs(mY))) * 2.; // rect distace from flipping sign in [0,1]
        float p = 6;
        float minDSmooth = pow(minD, 1. / p);
        float v = (mX * mY); // in [-1, 1], color switches at 0
        float adjV = sign(v) * minDSmooth;
        float s = smoothstep(-1.f, 1.f, adjV);
        albedoColor = mix(albedoColor, albedoColorDark, s);
      )"}
    },
    /* uniforms */ {
       {"u_modLen", DataType::Float},
       {"u_modDarkness", DataType::Float},
    },
    /* attributes */ {},
    /* textures */ {}
);

// input shadeValue2
//output albedoColor
const ShaderReplacementRule PARAM_TEXT2COLOR (
    /* rule name */ "PARAM_TEXT2COLOR",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform float u_modLen;
          uniform float u_angle;
          uniform bool u_flip;
          uniform bool v_flip;
          uniform sampler2D t_image;
        )"},
      {"GENERATE_SHADE_COLOR", R"(

        float sin_factor = sin(u_angle);
        float cos_factor = cos(u_angle);

        if(u_flip) {
          shadeValue2.x = -shadeValue2.x;
        }
        if(v_flip) {
          shadeValue2.y = -shadeValue2.y;
        }

        shadeValue2 = shadeValue2 * mat2(cos_factor, -sin_factor, sin_factor, cos_factor);
        shadeValue2 = shadeValue2 * u_modLen;

        vec3 albedoColor = texture(t_image, shadeValue2).rgb;

      )"}
    },
    /* uniforms */ {
        {"u_modLen", DataType::Float},
        {"u_angle", DataType::Float},
        {"u_flip", DataType::Int},
        {"v_flip", DataType::Int},
    },
    /* attributes */ {},
    /* textures */ { {"t_image", 2} }
);



const ShaderReplacementRule GENERATE_WORLD_POS (
    /* rule name */ "GENERATE_WORLD_POS",
    { /* replacement sources */
      {"FRAG_DECLARATIONS", R"(
          uniform mat4 u_invProjMatrix_worldPos; // weird names are to unique-ify because we have multiple....
          uniform mat4 u_invViewMatrix_worldPos;
          uniform vec4 u_viewport_worldPos;
          vec3 fragmentViewPosition(vec4 viewport, vec2 depthRange, mat4 invProjMat, vec4 fragCoord);
        )"},
      {"GLOBAL_FRAGMENT_FILTER", R"(
        vec2 depthRange_worldPos = vec2(gl_DepthRange.near, gl_DepthRange.far);
        vec4 fragCoord_worldPos = gl_FragCoord;
        fragCoord_worldPos.z = depth;
        vec4 worldPos4 = u_invViewMatrix_worldPos * vec4(fragmentViewPosition(u_viewport_worldPos, depthRange_worldPos, u_invProjMatrix_worldPos, fragCoord_worldPos), 1.);
        vec3 worldPos = worldPos4.xyz / worldPos4.w;
      )"}
    },
    /* uniforms */ {
      {"u_invProjMatrix_worldPos", DataType::Matrix44Float},
      {"u_invViewMatrix_worldPos", DataType::Matrix44Float},
      {"u_viewport_worldPos", DataType::Vector4Float},
    },
    /* attributes */ {},
    /* textures */ {}
);

const ShaderReplacementRule CULL_POS_FROM_WORLD (
    /* rule name */ "CULL_POS_FROM_WORLD",
    { /* replacement sources */
      {"GLOBAL_FRAGMENT_FILTER", R"(
        vec3 cullPos = worldPos;
      )"}
    },
    /* uniforms */ {},
    /* attributes */ {},
    /* textures */ {}
);

const ShaderReplacementRule CULL_POS_FROM_ATTR (
    /* rule name */ "CULL_POS_FROM_ATTR",
    { /* replacement sources */
      {"GLOBAL_FRAGMENT_FILTER", R"(
        vec3 cullPos = cullPosAttr;
      )"}
    },
    /* uniforms */ {},
    /* attributes */ {},
    /* textures */ {}
);


ShaderReplacementRule generateSlicePlaneRule(std::string uniquePostfix) {

  std::string centerUniformName = "u_slicePlaneCenter_" + uniquePostfix;
  std::string normalUniformName = "u_slicePlaneNormal_" + uniquePostfix;

  // This takes what is otherwise a simple rule, and substitues uniquely named uniforms so that we can have multiple slice planes
  ShaderReplacementRule slicePlaneRule (
      /* rule name */ "SLICE_PLANE_CULL_" + uniquePostfix,
      { /* replacement sources */
        {"FRAG_DECLARATIONS", "uniform vec3 " + centerUniformName + "; uniform vec3 " + normalUniformName + ";"},
        {"GLOBAL_FRAGMENT_FILTER",
         "if(dot(cullPos, " + normalUniformName + ") < dot( " + centerUniformName + " , " + normalUniformName + ")) { discard; }"}
      },
      /* uniforms */ {
        {centerUniformName, DataType::Vector3Float},
        {normalUniformName, DataType::Vector3Float},
      },
      /* attributes */ {},
      /* textures */ {}
  );

  return slicePlaneRule;
}


// clang-format on

} // namespace backend_openGL3_glfw
} // namespace render
} // namespace polyscope
