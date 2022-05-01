using UnityEngine;

[CreateAssetMenu(fileName = "New model", menuName = "Assets/Model")]

public class Model : ScriptableObject {
    public string modelID;
    public string registrationNumber;
    public string modelName;
    public GameObject model;

}
