#include <Gfx/Graph/NodeRenderer.hpp>
#include <Gfx/Graph/RenderList.hpp>

#include <score/tools/Debug.hpp>
#include <ossia/detail/algorithms.hpp>

namespace score::gfx
{

#include <Gfx/Qt5CompatPush> // clang-format: keep

TextureRenderTarget GenericNodeRenderer::renderTargetForInput(const Port& p)
{
  SCORE_TODO;
  return { };
}

void GenericNodeRenderer::defaultMeshInit(RenderList& renderer)
{
  if (!m_meshBuffer)
  {
    const auto& mesh = node.mesh();
    auto [mbuffer, ibuffer] = renderer.initMeshBuffer(mesh);
    m_meshBuffer = mbuffer;
    m_idxBuffer = ibuffer;
    if(m_meshBuffer)
      m_meshBuffer->setName("GenericNodeRenderer::m_meshBuffer");
    if(m_idxBuffer)
      m_idxBuffer->setName("GenericNodeRenderer::m_idxBuffer");
  }
}

void GenericNodeRenderer::defaultUBOInit(RenderList& renderer)
{
  auto& rhi = *renderer.state.rhi;
  m_processUBO = rhi.newBuffer(
      QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(ProcessUBO));
  m_processUBO->setName("GenericNodeRenderer::m_processUBO");
  m_processUBO->create();
}

void GenericNodeRenderer::defaultPassesInit(RenderList& renderer)
{
  for(Edge* edge : this->node.output[0]->edges)
  {
    auto rt = renderer.renderTargetForOutput(*edge);
    if(rt.renderTarget)
    {
      m_p.emplace_back(edge, score::gfx::buildPipeline(
          renderer,
          node.mesh(),
          node.m_vertexS,
          node.m_fragmentS,
          rt,
          m_processUBO,
          m_material.buffer, m_samplers));
    }
  }
}


void GenericNodeRenderer::init(RenderList& renderer)
{
  defaultMeshInit(renderer);
  defaultUBOInit(renderer);

  m_material.init(renderer, node.input, m_samplers);

  defaultPassesInit(renderer);
}

void GenericNodeRenderer::defaultUBOUpdate(RenderList& renderer, QRhiResourceUpdateBatch& res)
{
  res.updateDynamicBuffer(
      m_processUBO, 0, sizeof(ProcessUBO), &this->node.standardUBO);

  if (m_material.buffer && m_material.size > 0
      && materialChangedIndex != node.materialChanged)
  {
    char* data = node.m_materialData.get();
    res.updateDynamicBuffer(m_material.buffer, 0, m_material.size, data);
    materialChangedIndex = node.materialChanged;
  }
}

void GenericNodeRenderer::update(
    RenderList& renderer,
    QRhiResourceUpdateBatch& res)
{
  defaultUBOUpdate(renderer, res);
}

void GenericNodeRenderer::defaultRelease(RenderList&)
{
  for (auto sampler : m_samplers)
  {
    delete sampler.sampler;
    // texture isdeleted elsewxheree
  }
  m_samplers.clear();

  delete m_processUBO;
  m_processUBO = nullptr;

  delete m_material.buffer;
  m_material.buffer = nullptr;

  for(auto& pass : m_p)
    pass.second.release();
  m_p.clear();

  m_meshBuffer = nullptr;
}

void NodeRenderer::runInitialPasses(
    RenderList&,
    QRhiCommandBuffer& commands,
    QRhiResourceUpdateBatch*& res,
    Edge& e)
{

}

void NodeRenderer::runRenderPass(
    RenderList&,
    QRhiCommandBuffer& commands,
    Edge& edge)
{

}
void GenericNodeRenderer::runRenderPass(
    RenderList& renderer,
    QRhiCommandBuffer& cb,
    Edge& edge)
{
  auto it = ossia::find_if(m_p, [ptr=&edge] (const auto& p){ return p.first == ptr; });
  SCORE_ASSERT(it != m_p.end());
  {
    const auto sz = renderer.state.size;
    cb.setGraphicsPipeline(it->second.pipeline);
    cb.setShaderResources(it->second.srb);
    cb.setViewport(QRhiViewport(0, 0, sz.width(), sz.height()));

    assert(this->m_meshBuffer);
    assert(this->m_meshBuffer->usage().testFlag(QRhiBuffer::VertexBuffer));
    node.mesh().setupBindings(*this->m_meshBuffer, this->m_idxBuffer, cb);

    cb.draw(node.mesh().vertexCount);
  }
}

void GenericNodeRenderer::release(RenderList& r)
{
  defaultRelease(r);
}

score::gfx::NodeRenderer::NodeRenderer() noexcept { }

score::gfx::NodeRenderer::~NodeRenderer() { }

#include <Gfx/Qt5CompatPop> // clang-format: keep

}
