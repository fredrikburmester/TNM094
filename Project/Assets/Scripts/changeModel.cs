using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Vuforia;

public class changeModel : MonoBehaviour
{
    // Start is called before the first frame update
    public AnchorBehaviour gps;

    void Start()
    {
        Button button = gameObject.GetComponent<Button>();
        button.onClick.AddListener(change);
        
    }

    // Update is called once per frame
    private void change()
    {
        if (gps.transform.childCount > 0)
        {
            GameObject child = gps.transform.GetChild(0).gameObject;
            Destroy(child);
        }
       
        GameObject model = Instantiate(Database.GetModelByRegNum("FDS435").model) as GameObject;
            
        model.transform.parent = gps.GetComponentInParent<Transform>();
        model.SetActive(true);
    }
}
