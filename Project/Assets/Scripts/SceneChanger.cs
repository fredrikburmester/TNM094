using UnityEngine;
using UnityEngine.SceneManagement;
public class SceneChanger : MonoBehaviour
{
    public void Varning()
    {
        SceneManager.LoadScene("Varning");
    }
    public void Ej_klar()
    {
        SceneManager.LoadScene("Ej_klar");
    }

}
