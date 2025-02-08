#version 330 core

layout(location = 0) out vec4 fragColor;
in vec2 texCoords;
in vec2 bounds;
uniform sampler2D texture1;

void main()
{
    float maxTheta = bounds.x;
    float maxPhi = bounds.y;
    float phi = texCoords.y
    float uncorrectedTheta = texCoords.x;
    float uncorrectedMaxTheta = maxTheta / maxPhi * phi;
    // uncorrected: theta ranges from [0, uncorrectedMaxTheta]
    // corrected: we map theta to [0, maxTheta]
    float correctedTheta = uncorrectedTheta / uncorrectedMaxTheta * maxTheta;
    float finalSample = vec2(correctedTheta, phi);
    fragColor = texture(texture1, finalSample);
};