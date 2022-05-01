using UnityEngine;
using UnityEngine.SceneManagement;

public class SplashScreen : MonoBehaviour
{
    [SerializeField]
    //Hur l�ng tid delayen �r tills n�sta scen (Varning) laddas
    private float delayBeforeLoading = 3f;
    [SerializeField]
    private string sceneNameToLoad;
    private float timeElapsed;
  

    private void Update()
    {
        timeElapsed += Time.deltaTime;

        if (timeElapsed > delayBeforeLoading)
        {
            SceneManager.LoadScene(sceneNameToLoad);
        }
    }
}