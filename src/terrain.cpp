#include "terrain.hpp"

#include <vector>
#include <cmath>
#include <complex>
#include <thread>
#include <memory>

#include <glad/glad.h>

#include "util.hpp"
#include "shader.hpp"

Terrain::Terrain()
{
    loadMesh(m_loadingOffset, m_scale, &m_currentMeshPoints);
    loadMesh(m_loadingOffset, m_scale, &m_loadingMeshPoints);

    m_mesh.setVertices(m_currentMeshPoints);
    m_loadingMesh.setVertices(m_currentMeshPoints);

    auto texture = std::make_shared<Texture>("textures/texture.png");
    m_mesh.setTexture(texture);
    m_loadingMesh.setTexture(texture);

    auto meshIndices = generateMeshIndices();
    m_mesh.setIndices(meshIndices);
    m_loadingMesh.setIndices(meshIndices);

    m_vertexShader.attachTo(m_shaderProgram);
    m_fragmentShaders[m_currentFragmentShader].attachTo(m_shaderProgram);
    m_shaderProgram.compile();

    startLoading();
}

Terrain::~Terrain()
{
    m_loadingProcess.wait();
}

auto
Terrain::shaderProgram() -> ShaderProgram&
{
    return m_shaderProgram;
};

auto
Terrain::handleMomentaryAction(MomentaryAction const& action) -> void
{
    auto onTrigger = [this](TriggerAction action) {
        switch(action) {
        case TriggerAction::IncreaseIterations: {
            m_iterations += 20;
        } break;
        case TriggerAction::DecreaseIterations: {
            m_iterations -= 20;
        } break;
        case TriggerAction::SwitchShader: {
            m_currentFragmentShader++;
            m_currentFragmentShader %= m_fragmentShaders.size();

            m_fragmentShaders[m_currentFragmentShader].attachTo(
                    m_shaderProgram);

            m_shaderProgram.compile();
        } break;
        default:
            break;
        }
    };

    std::visit(util::Overload{onTrigger, util::unaryNOP}, action);
}

auto
toGpuVec(glm::dvec3 const v) -> glm::vec3
{
    return static_cast<glm::vec3>(v);
}

auto
Terrain::loadMesh(
        glm::dvec3 offset,
        double const scale,
        std::vector<glm::vec3>* const buffer) -> void
{
    // The mesh points need to be clamped in such a way that
    // reloading yields a smooth transition without big jumps.

    auto constexpr nrVertices = granularity * granularity;

    if(buffer->size() != nrVertices) {
        buffer->resize(nrVertices);
    }

    auto constexpr doublingInterval = 40;

    // Take longer steps for indices far away from middle
    auto const unscaledStepSize = [](int i) {
        return std::pow(2.0, std::abs(i - granularity / 2) / doublingInterval);
    };

    // CPP20 use ranges
    double const unscaledMeshSize = [&unscaledStepSize]{
        auto sum = 0.0;
        for(int i = 0; i < granularity; ++i) {
            sum += unscaledStepSize(i);
        }
        return sum;
    }();

    auto const quantizedScale = std::pow(2.0, int(log2(scale)));
    auto const meshSize  = 300.0 / quantizedScale;

    auto const scaleFactor   = meshSize / unscaledMeshSize;
    auto const stepSize = [scaleFactor, &unscaledStepSize](int i) {
        return scaleFactor * unscaledStepSize(i);
    };

    // Quantize x to grid, with distance stepSize between gridlines.
    auto const quantized = [](double x, double stepSize) {
        return std::floor(x / stepSize) * stepSize;
    };
    
    glm::vec3 const gpuOffset = toGpuVec(offset);

    auto xPos = -meshSize / 2 + offset.x;
    for(int x = 0; x < granularity; ++x) {
        auto const xQuant = quantized(xPos, stepSize(x));

        auto zPos = -meshSize / 2 + offset.z;
        for(int z = 0; z < granularity; ++z) {
            auto const zQuant              = quantized(zPos, stepSize(z));
            (*buffer)[x * granularity + z] = glm::vec3(
                    xQuant - gpuOffset.x,
                    heightAt({xQuant, zQuant}),
                    zQuant - gpuOffset.z);

            zPos += stepSize(z);
        }
        xPos += stepSize(x);
    }
}

auto
Terrain::startLoading() -> void
{
    m_loadingProcess = std::async(std::launch::async, [this]() {
        loadMesh(m_loadingOffset, m_scale, &m_loadingMeshPoints);
    });
}

auto
Terrain::updateMesh(double const x, double const z, double const scale)
        -> glm::dvec3
{
    auto const uploadSize = std::min(
            uploadChunkSize,
            (int)(m_currentMeshPoints.size() - m_loadIndex));
    m_loadingMesh.setVertices(m_currentMeshPoints, m_loadIndex, uploadSize);
    m_loadIndex += uploadSize;

    auto const uploadingDone = m_loadIndex >= (int)m_currentMeshPoints.size();

    if(uploadingDone) {
        switch(m_state) {
        case State::Loading: {
            if(util::isDone(m_loadingProcess)) {
                std::swap(m_currentMeshPoints, m_loadingMeshPoints);
                m_loadIndex = 0;

                m_state = State::Uploading;
            }
        } break;

        case State::Uploading: {
            swap(m_mesh, m_loadingMesh);

            m_offset        = m_loadingOffset;
            m_loadingOffset = {x, 0.0, z};
            m_scale         = scale;

            startLoading();

            m_state = State::Loading;
        } break;
        }
    }

    //offset must be consistent with shader offset (32 bit float)
    return toGpuVec(m_offset);
}

auto
Terrain::generateMeshIndices() -> std::vector<GLuint>
{
    std::vector<GLuint> meshIndices;
    meshIndices.reserve(granularity * granularity * 6);

    for(int x = 0; x < granularity - 1; x++) {
        for(int z = 0; z < granularity - 1; z++) {
            meshIndices.push_back(z + x * granularity);
            meshIndices.push_back((z + 1) + x * granularity);
            meshIndices.push_back(z + (x + 1) * granularity);

            meshIndices.push_back((z + 1) + x * granularity);
            meshIndices.push_back((z + 1) + (x + 1) * granularity);
            meshIndices.push_back(z + (x + 1) * granularity);
        }
    }

    return meshIndices;
}

auto
Terrain::heightAt(glm::dvec2 const& pos) -> double
{
    auto c  = std::complex<double>(pos.x, pos.y);
    auto z  = std::complex<double>(0.0, 0.0);
    auto dz = std::complex<double>(0.0, 0.0);

    // main cardioid check
    auto const q = pow(c.real() - 0.25, 2.0) + c.imag() * c.imag();
    if(q * (q + (c.real() - 0.25)) < 0.25 * c.imag() * c.imag()) {
        return 0.0;
    }

    // period-2 bulb check
    if((c.real() + 1.0) * (c.real() + 1.0) + c.imag() * c.imag()
       < 0.25 * 0.25) {
        return 0.0;
    }

    for(int i = 0; i < m_iterations; ++i) {
        dz = 2.0 * z * dz + 1.0;
        z  = z * z + c;

        if(std::abs(z) > 256) {
            auto const r                  = std::abs(z);
            auto const dr                 = std::abs(dz);
            auto const distanceEstimation = 2.0 * r * std::log(r) / dr;

            return distanceEstimation;
        }
    }

    return 0.0;
}

auto
Terrain::render() -> void
{
    m_shaderProgram.setUniformInt("iterations", m_iterations);
    m_shaderProgram.setUniformVec2("offset", {m_offset.x, m_offset.z});

    m_mesh.render();
}
