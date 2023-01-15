#version 450 core

#define MAX_ITER 512

out vec4 FragColor;

in vec2 TexCoord;

void main()
{
	vec2 p = TexCoord * vec2(2.47,2.24) - vec2(2.00,1.12);
	vec2 a = vec2(0,0);
	vec2 b = vec2(0,0);
	int iter = 0;
	while (b.x + b.y <= 4 && iter < MAX_ITER)
	{
		a.y = (a.x + a.x) * a.y + p.y;
		a.x = b.x - b.y + p.x;
		b.x = a.x * a.x;
		b.y = a.y * a.y;
		iter++;
	}
	if (iter >= MAX_ITER)
	{
		FragColor = vec4(0);
		return;
	}
	float red = float(iter) / MAX_ITER;
	float green = iter <= MAX_ITER / 2 ? float(iter + iter) / MAX_ITER : 1 - (float(iter + iter) / MAX_ITER - 1);
	float blue = 1 - float(iter) / MAX_ITER;
	FragColor = vec4(red, green, blue, 1);
}