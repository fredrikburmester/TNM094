using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;

public class Database : MonoBehaviour {
    public ModelDatabase models;

    private static Database instance;

    private void Awake()
    {
        if (instance == null)
        {
            instance = this;
            DontDestroyOnLoad(gameObject);
        }
        else
        {
            Destroy(gameObject);
        }
    }

    public static Model GetModelByID(string ID)
    {
        return instance.models.allModels.FirstOrDefault(i => i.modelID == ID);
    }
    public static Model GetModelByRegNum(string RegNum)
    {
        return instance.models.allModels.FirstOrDefault(i => i.registrationNumber == RegNum);
    }
}
