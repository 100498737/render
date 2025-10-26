#pragma once
#include <functional>
#include <string>

namespace render {

// Escribe un PPM (ASCII P3) aplicando gamma 2.0 a cada píxel leído vía 'sampler'.
// 'sampler(x,y,r,g,b)' debe devolver r,g,b en [0,1] por referencia.
// Devuelve true si se pudo escribir correctamente.
bool write_ppm_gamma(const std::string& path, int width, int height,
                     const std::function<void(int,int,double&,double&,double&)>& sampler);

} // namespace render
