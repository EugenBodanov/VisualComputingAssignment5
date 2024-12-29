#version 330 core

// ---------- Структуры ----------

// Структура с параметрами освещения
struct Light {
    vec3 lightPos;                 // Положение источника
    vec3 globalAmbientLightColor;  // Цвет глобального фонового освещения
    vec3 lightColor;               // Цвет освещения (точечный / направленный источник)
    float ka;                      // Ambient  coefficient [0..1]
    float kd;                      // Diffuse  coefficient [0..1]
    float ks;                      // Specular coefficient [0..1]
};

// ---------- Юниформы ----------

uniform Light  uLight;
uniform vec3   uCameraPos;     // Позиция камеры в мировом пространстве

// Матрица для трансформации нормалей в мировое пространство:
//   n_world = (transpose(inverse(uModel))) * n_object
uniform mat4   uModel;         // Матрица M
// Можно сразу передавать инвертированную-транспонированную матрицу,
// тогда в коде шейдера ниже будет чуть короче. Но здесь оставим как есть.

uniform sampler2D map_diffuse;   // Текстура диффузного цвета
uniform sampler2D map_ambient;   // Текстура ambient-occlusion (AO)
uniform sampler2D map_specular;  // Текстура цвета для бликов (specular color)
uniform sampler2D map_shininess; // Текстура для shininess (Ns)
uniform sampler2D map_emission;  // Текстура для эмиссии
uniform sampler2D map_normal;    // Текстура нормалей (в объектном пространстве)

// ---------- Входные данные из вертексного шейдера ----------

// Положение фрагмента в мировом пространстве
in vec3 tFragPos;

// Нормаль, интерполированная от вершин (уже в мировом пространстве),
// но может пригодиться для смешивания с нормалью из карты (особенно для флага).
in vec3 tNormal; 

// Текстурные координаты
in vec2 TexCoords;

// ---------- Выход фрагментного шейдера ----------
out vec4 FragColor;

// ---------- Функции ----------

// Функция для расчёта освещения Blinn-Phong с учётом уже вычисленных
// (или откорректированных) нормалей в мировом пространстве
vec3 blinnPhongIllumination(
    vec3 normal, 
    vec3 fragPos, 
    vec3 cameraPos, 
    vec3 lightPos,
    vec3 ambientMaterial, 
    vec3 diffuseMaterial, 
    vec3 specularMaterial,
    float shininess
) {
    // Направление на источник
    vec3 lightDir = normalize(lightPos - fragPos);

    // Направление на камеру
    vec3 viewDir = normalize(cameraPos - fragPos);

    // Полупуть (halfway vector)
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // ---------------- Ambient ----------------
    // Умножаем на глобальный ambient-цвет и коэффициент ka
    vec3 ambientComponent =
        uLight.ka * ambientMaterial * uLight.globalAmbientLightColor;

    // ---------------- Diffuse ----------------
    // Множитель dot(normal, lightDir)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuseComponent =
        uLight.kd * diffuseMaterial * uLight.lightColor * diff;

    // ---------------- Specular ---------------
    // Бликовая составляющая
    float specAngle = max(dot(normal, halfwayDir), 0.0);
    float specFactor = pow(specAngle, shininess);
    vec3 specularComponent =
        uLight.ks * specularMaterial * uLight.lightColor * specFactor;

    return ambientComponent + diffuseComponent + specularComponent;
}

void main(void)
{
    // 1) Считываем данные из текстур
    // -------------------------------------------
    // Диффузный цвет (RGB), альфа = 1 при отсутствии канала
    vec3 diffuseColor = texture(map_diffuse, TexCoords).rgb;

    // Значение ambient-occlusion (AO). Обычно достаточно одного канала (r),
    // но т.к. текстура RGBA, берём .r
    float aoValue = texture(map_ambient, TexCoords).r;

    // Цвет для бликов
    vec3 specularColor = texture(map_specular, TexCoords).rgb;

    // Значение shininess (Ns). Тоже обычно берём .r.
    // По условию умножаем на 1000
    float shininess = texture(map_shininess, TexCoords).r * 1000.0;

    // Эмиссия
    vec3 emissionColor = texture(map_emission, TexCoords).rgb;

    // Нормаль из карты (в объектном пространстве):
    //   (0..1) -> (−1..1)
    vec3 normalMapValue = texture(map_normal, TexCoords).rgb;
    vec3 n_objectSpace  = normalMapValue * 2.0 - 1.0; // перевод в диапазон [-1..1]

    // 2) Трансформируем normal из объектного пространства в мировое
    // -------------------------------------------------------------
    // n_world = normalize( (M^-1)^T * n_objectSpace )
    // Для матриц/векторов используем vec4, поскольку умножаем на матрицу
    vec3 n_world = normalize( mat3(transpose(inverse(uModel))) * n_objectSpace );

    // 3) Для флагов (если бы это был флаг), нормаль можно было бы смешать
    //    с текущей нормалью tNormal. Пример:
    //
    //    float s = 0.25;
    //    vec3 n_simplified = normalize(s * n_world + (1.0 - s) * tNormal);
    //
    //    // Проверка направления относительно вектора взгляда
    //    float d = dot(tNormal, normalize(uCameraPos - tFragPos));
    //    if (d < 0.0) {
    //        n_simplified = -n_simplified;
    //    }
    //
    //    // И использовать n_simplified в расчёте. 
    //
    // В обычном случае (планета, плоскость) — просто n_world.
    // -------------------------------------------------------------

    // 4) Подготовим данные для Blinn-Phong
    // -------------------------------------------------------------
    // Согласно условию, ambient-цвет материала можно получить, 
    // умножив диффузный цвет на aoValue:
    vec3 ambientMaterial = diffuseColor * aoValue;

    // diffuseColor уже есть (из map_diffuse)
    // specularColor уже есть (из map_specular)

    // 5) Считаем итоговое освещение
    // -------------------------------------------------------------
    vec3 blinnResult = blinnPhongIllumination(
        n_world,
        tFragPos,
        uCameraPos,
        uLight.lightPos,
        ambientMaterial,
        diffuseColor,
        specularColor,
        shininess
    );

    // Добавляем эмиссию (emissionColor).
    // По условию, emission — это просто добавка к цвету (без умножения на что-либо).
    vec3 finalColor = blinnResult + emissionColor;

    // 6) Выдаём итоговый цвет фрагмента
    // -------------------------------------------------------------
    // Если нужна поддержка альфа-канала из map_diffuse, можно взять .a,
    // но чаще всего он = 1 (или используется для других нужд).
    float alpha = texture(map_diffuse, TexCoords).a;

    FragColor = vec4(finalColor, alpha);
}
