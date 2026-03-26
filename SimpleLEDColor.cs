using UnityEngine;

public class SimpleLEDColor : MonoBehaviour
{
    [Header("LED Setup")]
    public int ledIndex = 0;
    public int totalLEDs = 16;

    [Header("Colors")]
    public Color defaultColor = Color.yellow;
    public Color waveColor = new Color(1f, 0.5f, 0f); // orange

    [Header("Wave Settings")]
    public float waveSpeed = 5f; // LEDs per second

    private static float wavePosition = 0f;
    private static bool wavingForward = true;

    private Renderer ledRenderer;

    void Start()
    {
        ledRenderer = GetComponent<Renderer>();
    }

    void Update()
    {
        // Only the first LED drives the wave position
        if (ledIndex == 0)
        {
            print("My name is " + gameObject.name + " and I am driving the wave.");
            if (wavingForward)
            {
                wavePosition += waveSpeed * Time.deltaTime;
                if (wavePosition >= totalLEDs - 1)
                    wavingForward = false;
            }
            else
            {
                wavePosition -= waveSpeed * Time.deltaTime;
                if (wavePosition <= 0)
                    wavingForward = true;
            }
        }

        // Bell-curve gradient: flat orange core (~2 LEDs), smooth fade on each side
        float dist = Mathf.Abs(wavePosition - ledIndex);
        float distFromCore = Mathf.Max(0f, dist - 0.5f);        // 0 inside core, grows outside
        float t = Mathf.SmoothStep(0f, 1f, distFromCore);       // smooth falloff over ~1 LED
        Color color = Color.Lerp(waveColor, defaultColor, t);

        if (ledRenderer is not null)
            ledRenderer.material.color = color;
    }
}
