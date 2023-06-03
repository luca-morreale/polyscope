#pragma once

#include "polyscope/affine_remapper.h"
#include "polyscope/render/engine.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/surface_parameterization_quantity.h"

namespace polyscope {

// forward declaration
class SurfaceMeshQuantity;
class SurfaceMesh;
class SurfaceParameterizationQuantity;

class SurfaceTextureQuantity : public SurfaceMeshQuantity,
                                public ParameterizationQuantity<SurfaceTextureQuantity> {
public:
  SurfaceTextureQuantity(std::string name, std::vector<glm::vec2> uvs, const Texture& texture, SurfaceMesh& mesh);

  virtual void draw() override;
  virtual std::string niceName() override;
  virtual void buildCustomUI() override;
  virtual void refresh() override;

  SurfaceTextureQuantity* setTexture(const Texture& texture);

private:
  std::shared_ptr<render::ShaderProgram> program;
  bool flip_uv_u = false;
  bool flip_uv_v = true;

  void createProgram();
  void setProgramUniforms(render::ShaderProgram& program);
  void fillColorBuffers(render::ShaderProgram& p);
};

} // namespace polyscope
