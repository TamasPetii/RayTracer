#version 460 core
#define MAX_SPHERE_NUMBER 250

struct Hit
{
	int type;
    int success;
	vec3 color;
	vec3 point;
	vec3 normal;
};

struct Ray
{
	vec3 origin;
	vec3 direction;
};

struct Sphere
{
	vec3 color;
	vec3 origin;
	float radius;
	int type;
};

//Input Data
in vec3 frag_position;

//Output Data
out vec4 out_color;

//Uniform Data
uniform mat4 uViewPorjMatrix;
uniform vec3 uLightDirection;
uniform vec3 uCameraEye;
uniform int uPathDepth;
uniform int uSphereCount;
uniform Sphere uSpheres[MAX_SPHERE_NUMBER];

float Intersection(Ray ray, Sphere sphere)
{
	// (t * RD + (RO - CO)) * (t * RD + (RO - CO)) = r^2
	// t^2 * RD * RD + 2*t*RD*(RO-CO) + (RO-CO)*(RO-CO) - r^2 = 0
	float a = dot(ray.direction, ray.direction);
	float b = 2 * dot(ray.direction, ray.origin - sphere.origin);
	float c = dot(ray.origin - sphere.origin, ray.origin - sphere.origin) - pow(sphere.radius, 2);

	float discriminant = b * b - 4 * a * c;

	if(discriminant >= 0)
	{
		float root1 = (-b + sqrt(discriminant)) / (2 * a);
		float root2 = (-b - sqrt(discriminant)) / (2 * a);

		if(root1 < 0) return root2;
		if(root2 < 0) return root1;
		if(root1 >= 0 && root1 <= root2) return root1;
		if(root2 >= 0 && root2 < root1) return root2;
	}

	return -1;
}

Hit ClosesHit(Ray ray)
{
	Hit hit;
	hit.success = 0;
	float hitMaxDistance = 1000;

	for(int i = 0; i < uSphereCount; i++)
	{
		float hitDistance = Intersection(ray, uSpheres[i]);

		if(hitDistance >= 0 && hitDistance < hitMaxDistance)
		{
			hitMaxDistance = hitDistance;
			hit.success = 1;
			hit.point = ray.origin + hitDistance * ray.direction;
			hit.normal = normalize(hit.point - uSpheres[i].origin);
			hit.color = uSpheres[i].color;
			hit.type = uSpheres[i].type;
		}
	}

	return hit;
}

Ray GenerateRay()
{
	vec4 startNdc = vec4(frag_position.xy, -1, 1);
	vec4 endNdc = vec4(frag_position.xy, 1, 1);
	vec4 startWorld = inverse(uViewPorjMatrix) * startNdc; 
	vec4 endWorld = inverse(uViewPorjMatrix) * endNdc;
	startWorld /= startWorld.w;
	endWorld /= endWorld.w;

	Ray ray;
	ray.origin = startWorld.xyz;
	ray.direction = endWorld.xyz - startWorld.xyz;

	return ray;
}

vec3 CalculateDirectionLight(Hit hit)
{
    vec3 toEye = normalize(uCameraEye - hit.point);
	vec3 toLight = normalize(-uLightDirection);
	vec3 normal = normalize(hit.normal);
	
	//Diffuse part
	float diffuseIntensity = dot(toLight, normal) * 0.5 + 0.5;
	vec3 diffuse = diffuseIntensity * hit.color;

	//Specular part
	vec3 specularReflection = normalize(reflect(-toLight, normal));
	float specularIntensity = pow(clamp(dot(toEye, specularReflection), 0, 1), 64);
	vec3 specular = specularIntensity * hit.color;

	return diffuse + specular;
}

void main()
{	
	out_color = vec4(0,0,0,1);

	Ray ray = GenerateRay();
	for(int i = 0; i < uPathDepth; i++)
	{
		Hit hit = ClosesHit(ray);

		if(hit.success == 1)
		{
			Ray newRay;
			newRay.origin = hit.point + hit.normal * 0.01;
			newRay.direction = reflect(ray.direction, hit.normal);
			ray = newRay;
			out_color += vec4(hit.color / pow(2, i) , 1);

			if(hit.type == 0)
			{
				out_color *= vec4(CalculateDirectionLight(hit), 1);
				i = uPathDepth;
			} 
		}
	}
}