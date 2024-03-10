//Here fragment shader is not mandatory but have this thing is better
//this thing controls colors of each side

#version 330

out vec4 colour;
in vec4 vCol;
in vec2 TexCoord;
in vec3 Normal;
in vec3 fragPos;

uniform vec3 lightColour;
uniform vec3 lightPos;

uniform sampler2D texture2D;

vec3 ambientLight()
{
    float ambientStrength = 0.2f;
    vec3 ambient = lightColour * ambientStrength;
    return ambient;
}

vec3 diffuseLight()
{
    float diffuseStrength = 0.5f;
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = lightColour * diff * diffuseStrength;
    return diffuse;
}

void main()
{
    colour = texture(texture2D, TexCoord) * vec4(ambientLight() + diffuseLight(), 1.0f);
}

