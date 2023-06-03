#include "polyscope/surface_texture_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/polyscope.h"
#include "polyscope/render/engine.h"

#include "stb_image.h"

#include "imgui.h"

#include <filesystem>
#include <fstream>

using std::cout;
using std::endl;

namespace polyscope {

SurfaceTextureQuantity::SurfaceTextureQuantity(std::string name, std::vector<glm::vec2> uvs, const Texture& texture, SurfaceMesh& mesh)
  : SurfaceMeshQuantity(name, mesh, true),
    ParameterizationQuantity(*this, uvs, ParamCoordsType::UNIT, ParamVizStyle::CHECKER) {
    checkerSize = 1.0;
    altDarkness = 1.0;
    setTexture(texture);
}


void SurfaceTextureQuantity::draw() {
  if (!isEnabled()) return;

  // Set uniforms
  setProgramUniforms(*program);
  parent.setStructureUniforms(*program);
  parent.setSurfaceMeshUniforms(*program);

  program->draw();
}

void SurfaceTextureQuantity::createProgram() {
  std::shared_ptr<render::ShaderProgram> tmpProgram = render::engine->requestShader("MESH",
                                          parent.addSurfaceMeshRules({"MESH_PROPAGATE_VALUE2", "SHADE_TEXTURE2COLOR"}));
  if (program != nullptr) {
    // If exists, grab textures from previous shader program before clearing it.
    tmpProgram->copyTextures(program);
    program.reset();
  }

  program = std::move(tmpProgram);

  fillColorBuffers(*program);
  parent.setMeshGeometryAttributes(*program);

  render::engine->setMaterial(*program, parent.getMaterial());
}


// Update range uniforms
void SurfaceTextureQuantity::setProgramUniforms(render::ShaderProgram& program) {
  program.setUniform("u_modLen", getCheckerSize());
  program.setUniform("u_modDarkness", getAltDarkness());
  program.setUniform("u_angle", localRot);
  program.setUniform("u_flip", (int)flip_uv_u);
  program.setUniform("v_flip", (int)flip_uv_v);
}

void SurfaceTextureQuantity::buildCustomUI() {

  ImGui::PushItemWidth(100);

  ImGui::SameLine(); // put it next to enabled

  // Modulo stripey width
  if (ImGui::DragFloat("scale", &checkerSize.get(), .01, 0.1, 5.0, "%.4f",
                       ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat)) {
    setCheckerSize(getCheckerSize());
  }


  ImGui::PopItemWidth();

  // Angle slider
  ImGui::PushItemWidth(100);
  ImGui::SliderAngle("rotation", &localRot, -180,
                     180); // displays in degrees, works in radians TODO refresh/update/persist
  if (ImGui::DragFloat("alt darkness", &altDarkness.get(), 0.01, 0., 1.)) {
    altDarkness.manuallyChanged();
    requestRedraw();
  }
  ImGui::PopItemWidth();
  ImGui::Checkbox("Flip u", &flip_uv_u);
  ImGui::SameLine();
  ImGui::Checkbox("Flip v", &flip_uv_v);

}

void SurfaceTextureQuantity::refresh() {
  createProgram();
  Quantity::refresh();
}

std::string SurfaceTextureQuantity::niceName() { return name; }

void SurfaceTextureQuantity::fillColorBuffers(render::ShaderProgram& p) {
  std::vector<glm::vec2> coordVal;
  coordVal.reserve(3 * parent.nFacesTriangulation());

  std::vector<glm::vec2> uvs_ = coordsData;

  for (size_t iF = 0; iF < parent.nFaces(); iF++) {

    size_t iStart  = parent.faceIndsStart[iF];
    uint32_t vRoot = parent.faceIndsEntries[iStart];

    size_t D = parent.faceIndsStart[iF + 1] - parent.faceIndsStart[iF];

    // implicitly triangulate from root
    for (size_t j = 1; (j + 1) < D; j++) {
      uint32_t vB = parent.faceIndsEntries[iStart + j];
      uint32_t vC = parent.faceIndsEntries[iStart + ((j + 1) % D)];

      coordVal.push_back(uvs_[vRoot]);
      coordVal.push_back(uvs_[vB]);
      coordVal.push_back(uvs_[vC]);
    }
  }
  // Store data in buffers
  p.setAttribute("a_value2", coordVal);
}

SurfaceTextureQuantity* SurfaceTextureQuantity::setTexture(const Texture& texture) {
  program.reset();
  createProgram();
  program->setTexture2D("t_image", texture.data, texture.width, texture.height, true, true, true);

  requestRedraw();
  return this;
}

} // namespace polyscope
