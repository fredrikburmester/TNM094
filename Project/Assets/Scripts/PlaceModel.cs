using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Vuforia;
using UnityEngine.UI;


public class PlaceModel : MonoBehaviour
{
    public PlaneFinderBehaviour plane;

    // Start is called before the first frame update
    void Start()
    {
        Button button = gameObject.GetComponent<Button>();
        button.onClick.AddListener(place);
    }

    // Update is called once per frame
    public void place()
    {
        Vector2 aPosition = new Vector2(0, 0);
        plane.PerformHitTest(aPosition);
    }
}
