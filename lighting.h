#pragma once

struct Light {
    unsigned char r = 0, g = 0, b = 0;
    float coeffR = 0.0f, coeffG = 0.0f, coeffB = 0.0f;
    int x = 0, y = 0, z = 0;

    Light(unsigned char r, unsigned char g, unsigned char b, int x = 0, int y = 0, int z = 0,
        float coeffR = 0.0f, float coeffG = 0.0f, float coeffB = 0.0f)
        : r(r), g(g), b(b), x(x), y(y), z(z), coeffR(coeffR), coeffG(coeffG), coeffB(coeffB) {}

    Light(float coeffR, float coeffG, float coeffB)
        : coeffR(coeffR), coeffG(coeffG), coeffB(coeffB) {}
};

struct Lighting {
    Light source, diffusion, reflection, ambient;

    Lighting(const Light& source, const Light& diffusion, const Light& reflection, const Light& ambient)
        : source(source), diffusion(diffusion), reflection(reflection), ambient(ambient) {}
};