using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


//using System.Runtime.InteropServices;
using System.Threading;
using System;

using System.Reflection;
using System.Text;
using System.Security;
using SkpInterface;
//使用call_me接口；
public class MyMesh : MonoBehaviour {

    //public FBXImporter fbxImporter;
    //用于存储绘制三角形的顶点坐标  
    private Vector3[] vertices;
    //用于记录绘制三角形所需要的顶点ID顺序  
    private int[] triangles;
    //记录顶点数  
    private int count = 0;
    //定义Mesh  
    private Mesh mesh;
    //定义一个链表用于记录所有点的坐标  
    private List<Vector3> list;

    Mesh  m_mesh;
    float m_speed = 10.0f;
    float m_rotationY = 0.0f;

    void Start()
    {

    
        MeshInfo tSingleMeshInfo = MeshInfo.getInstance();
        //liuliang测试直接运行场景测试
        tSingleMeshInfo.getmeshData().FileType = "SKP";
        //测试带纹理模型
        //tSingleMeshInfo.meshData.PathName = Application.dataPath + "/Resources/uv_test1cube/TextureBox.skp";
        // tSingleMeshInfo.meshData.FilePath = Application.dataPath + "/Resources/uv_test1cube";

        //测试颜色模型
        tSingleMeshInfo.meshData.PathName = "D:/sketchup/cube.skp";
        tSingleMeshInfo.meshData.FilePath = "D:/sketchup/reader_data";
        //liuliang测试直接运行场景测试
        if (tSingleMeshInfo.getmeshData().FileType == "DEF")
        {
            GameObject  newObject = Instantiate(Resources.Load("DefaultMesh")) as GameObject;
            newObject.transform.localScale = new Vector3(1.0f, 1.0f, 1.0f);
            newObject.transform.position = new Vector3(0.0f, 0.0f, 0.0f);
            return;
        }
      
        if (tSingleMeshInfo.getmeshData().FileType == "SKP")
        {

            IntPtr _skp_exporter = IntPtr.Zero;
            _skp_exporter = SkpInterface.SkpDLL.GetExporter(tSingleMeshInfo.meshData.PathName, tSingleMeshInfo.meshData.FilePath);
            Debug.Log("加载模型2");
        

            if (_skp_exporter.ToInt32() > 0)
            {
                unsafe
                {
                    double* _vertices;
                    int _vertex_num;
                    int* _vertex_num_per_face;
                    int _face_num;
                    int* _face_vertex_index;

                    SkpInterface.SkpDLL.GetSkpFaceData(_skp_exporter,
                                out _vertices,
                                out _vertex_num,
                                out _vertex_num_per_face,
                                out _face_num,
                                out _face_vertex_index);
                    Debug.Log("vertex num =" + _vertex_num);
                    Debug.Log("face num =" + _face_num);


                    gameObject.AddComponent<MeshFilter>();
                    gameObject.AddComponent<MeshRenderer>();
                    Mesh mesh = GetComponent<MeshFilter>().mesh;
                    mesh.Clear();

                   
                    Vector3[] zyVector = new Vector3[_vertex_num];
                    int[] zyFaceVector = new int[_face_num*3];
                    int i = 0;
                    //填充顶点信息
                    while (i < _vertex_num)
                    {
                        zyVector[i] = new Vector3((float)_vertices[i * 3], (float)_vertices[i * 3+1], (float)_vertices[i * 3+2]);
                        Debug.Log("zyVector ： " + zyVector[i]);
                        i++;
                    }
                    //填充顶点索引信息
                    int j = 0;
                    while (j < _face_num)
                    {
                        zyFaceVector[j*3] = _face_vertex_index[j*3];
                        zyFaceVector[j * 3 + 1] = _face_vertex_index[j * 3 + 1];
                        zyFaceVector[j * 3 + 2] = _face_vertex_index[j * 3 + 2];

                        j++;
                    }
                    //填充法线信息
                    //
                    Vector3[] ZYnormals = new Vector3[_vertex_num];
                    int k = 0;
                    //填充法线信息
                    while (k < _vertex_num)
                    {
                        ZYnormals[k] = new Vector3((float)_vertices[k * 3], (float)_vertices[k * 3 + 1], (float)_vertices[k * 3 + 2]);
                        k++;
                    }

                    mesh.vertices = zyVector;
                    mesh.normals = ZYnormals;//法线
                    mesh.triangles = zyFaceVector;
                    mesh.RecalculateNormals();
                    mesh.RecalculateBounds();
                    //构造子模型数

                    Debug.Log("法线数");




                    //uv test-----------------------------------------------------
                    double* u;//u value list
                    double* v;//v value list
                    int uv_num = 0;//u v list length
                    int* uv_id;//the index of uv for each face-vertex
                    int uv_id_num = 0; //the length of uv_id
                    SkpInterface.SkpDLL.GetSkpFaceUV(_skp_exporter,
                                                                            true,//true for front uv,false for back uv
                                                                            out u,
                                                                            out v,
                                                                            out uv_num,
                                                                            out uv_id,
                                                                            out uv_id_num);
                    Debug.Log("uv_num ： " + uv_num);
                    Debug.Log("uv_id_num ： " + uv_id_num);
                    //liuliang解析模型UV坐标和UV索引
                    Vector2[] uv = new Vector2[uv_num];
                    for (int y = 0; y < uv.Length; y++)
                    {
                        uv[y] = new Vector2((float)u[y], (float)v[y]);
                        Debug.Log(y+"UI坐标" + uv[y]);
                       
                    }
                    int[] uvIndex = new int[uv_id_num];
                    for(int h = 0; h<uv_id_num ; h++)
                    {
                        uvIndex[h] = uv_id[h];
                       
                    }

                    Vector2[] meshuv = new Vector2[_vertex_num];
                    for (int v2 = 0; v2 < uv_id_num; v2++)
                    {
                        int Vindex = zyFaceVector[v2];
                        //Debug.Log("Vindex ： " + Vindex);
                        int UVindex  = uvIndex[v2];
                        if (UVindex>=0)
                        {

                            meshuv[Vindex] = new Vector2(uv[UVindex].x, uv[UVindex].y);
                            //Debug.Log("UVindex ： " + Vindex);
                            
                        }
                        else
                        {
                            meshuv[Vindex] = new Vector2(0.0f, 0.0f);
                        }
                        
                    }
                    //liuliang测试UV  
                    meshuv[0] = new Vector2(0.0f, 0.0f);
                    meshuv[1] = new Vector2(1.0f, 1.0f);
                    meshuv[2] = new Vector2(1.0f, 0.0f);
                    meshuv[3] = new Vector2(0.0f, 1.0f);
                    mesh.uv = meshuv;
                    //liuliang测试UV
                    ////material test---------------------------------------------------
                    int mat_num = SkpInterface.SkpDLL.GetMaterialNum(_skp_exporter);//get the num of materials
                    Debug.Log("yj_mat_num ： " + mat_num);
                    //构造材质
                    if (mat_num>0)
                    {
                        this.gameObject.GetComponent<MeshRenderer>().materials = new Material[mat_num];
                        for (int m = 0; m < mat_num; m++)
                        {
                            float r = UnityEngine.Random.Range(0.0f, 1.0f);
                            float g = UnityEngine.Random.Range(0.0f, 1.0f);
                            float b = UnityEngine.Random.Range(0.0f, 1.0f);
                            Color color = new Color(r, g, b);
                            this.gameObject.GetComponent<MeshRenderer>().materials[m].color = color;
                            //动态读取材质纹理根据材质路径
                            //StartCoroutine(IGetSKPTexture("", m));  
                        }
                    }
                       
                    int mat_index = 0;
                    while (mat_index < mat_num)
                    {
                        StringBuilder mat_name = new StringBuilder(100);

                        SkpInterface.SkpDLL.GetMaterialNameByID(_skp_exporter, mat_index, mat_name); //get the material name by id
                        //string str = new string(materialName);
                        Debug.Log("material name -------" + mat_name.ToString());

                        bool has_color = false;
                        double []color = new double[3];
                        bool has_alpha = false;
                        double alpha = 1;
                        bool has_texture = false;
                        StringBuilder tex_path = new StringBuilder(100);
                        double tex_sscale = 1.0;
                        double tex_tscale = 1.0;
                        StringBuilder tex_p = new StringBuilder();
                        SkpInterface.SkpDLL.GetMaterialData(_skp_exporter,
                                 mat_index,
                                 out has_color,
                                 color,
                                 out has_alpha,
                                 out alpha,
                                 out has_texture,
                                 tex_path,
                                 out tex_sscale,
                                 out tex_tscale);

                        //Debug.Log("materail -------"+ mat_index);

                        if (has_color)
                        {
                              Debug.Log("Color -------"+ mat_index + " "+color[0]+" "+color[1]+" "+color[2]);

                        }
                        if (has_alpha)
                        {
                            Debug.Log("Alpha-------" + mat_index + " " + alpha);
                        }
                        if (has_texture)
                        {
                            //如果纹理存在，应该打印出纹理路径
                            Debug.Log("Texture------" + mat_index + " " + tex_path.ToString());
                        }
                        mat_index++;
                      
         
                    }


                    //----------------------------------------------------------------------
                    int* front_mat_id;//front material
                    int* back_mat_id;//
                    int id_num = 0;
                    SkpInterface.SkpDLL.GetSkpMaterialIDPerFace(_skp_exporter,
                                                                                            out front_mat_id,
                                                                                            out back_mat_id,
                                                                                            out id_num);


                    int mat_index_per_face = 0;
                    Debug.Log("mat id per face:" + id_num);
                    Debug.Log("mesh.triangles:" + mesh.triangles.Length);
                    int[] triangles = mesh.triangles;
                    //设置子模型数目
                    mesh.subMeshCount = id_num;
                    while (mat_index_per_face < id_num)
                    {
                        Debug.Log(front_mat_id[mat_index_per_face]);
                        //设置子模型
                        mesh.SetTriangles(GetRangeArray(triangles, mat_index_per_face * 3, mat_index_per_face * 3 + 2), mat_index_per_face);
                        mat_index_per_face++;
                       


                    }

                   
                    //mesh.
                    Debug.Log("加载模型3");
                    gameObject.AddComponent<MeshCollider>();


                }

                //print something...
            }

           
            SkpInterface.SkpDLL.ReleaseExporter(_skp_exporter);
        }

        if (tSingleMeshInfo.getmeshData().FileType == "FBX")
        {
           // fbxImporter.ImportAllFBX(tSingleMeshInfo.meshData.PathName);
        }
        return;
        ////添加MeshFilter  
        //gameObject.AddComponent<MeshFilter>();
        ////添加MeshRenderer  
        //gameObject.AddComponent<MeshRenderer>();
        ////new一个链表  
        //list = new List<Vector3>();
        ////获得Mesh  
        //mesh = GetComponent<MeshFilter>().mesh;
        ////修改Mesh的颜色  
        //GetComponent<MeshRenderer>().material.color = Color.green;
        ////选择Mesh中的Shader  
        //GetComponent<MeshRenderer>().material.shader = Shader.Find("Transparent/Diffuse");
        ////清空所有点，用于初始化！  
        //mesh.Clear();
        //MeshFilter  _meshFilter = gameObject.AddComponent(typeof(MeshFilter)) as MeshFilter;//网格过滤器  
        //MeshRenderer _meshRenderer  = gameObject.AddComponent(typeof(MeshRenderer)) as MeshRenderer;//网格渲染  
        //m_mesh = _meshFilter.mesh as Mesh;    //网格过滤器的实例化的Mesh  

        ////u3d中绘制图形都是三角形绘制法，顺时针，所以先记录所有的点，然后在确定三角形绘制的顺序  

        //Vector3[] pVector = new Vector3[36];
        //Vector3[] pNormalArray = new Vector3[36];
        //Vector2[] pUVArry = new Vector2[36];
        //Color[] pColorArry = new Color[36];
        //int[] pTriangles = new int[pVector.Length];


        ////forword  
        //pVector[0] = new Vector3(0.0f, 0.0f, 0.0f);
        //pVector[1] = new Vector3(0.0f, 1.0f, 0.0f);
        //pVector[2] = new Vector3(1.0f, 0.0f, 0.0f);
        //pNormalArray[0] = new Vector3(0.0f, 0.0f, 0.0f);
        //pNormalArray[1] = new Vector3(0.0f, 1.0f, 0.0f);
        //pNormalArray[2] = new Vector3(1.0f, 0.0f, 0.0f);
        //pUVArry[0] = new Vector2(0.0f, 1.0f);
        //pUVArry[1] = new Vector2(0.0f, 0.0f);
        //pUVArry[2] = new Vector2(1.0f, 1.0f);
        //pColorArry[0] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[1] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[2] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        //pVector[3] = new Vector3(1.0f, 0.0f, 0.0f);
        //pVector[4] = new Vector3(0.0f, 1.0f, 0.0f);
        //pVector[5] = new Vector3(1.0f, 1.0f, 0.0f);
        //pNormalArray[3] = new Vector3(1.0f, 0.0f, 0.0f);
        //pNormalArray[4] = new Vector3(0.0f, 1.0f, 0.0f);
        //pNormalArray[5] = new Vector3(1.0f, 1.0f, 0.0f);
        //pUVArry[3] = new Vector2(1.0f, 1.0f);
        //pUVArry[4] = new Vector2(0.0f, 0.0f);
        //pUVArry[5] = new Vector2(1.0f, 0.0f);
        //pColorArry[3] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[4] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[5] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        ////back  
        //pVector[6] = new Vector3(1.0f, 0.0f, 1.0f);
        //pVector[7] = new Vector3(1.0f, 1.0f, 1.0f);
        //pVector[8] = new Vector3(0.0f, 0.0f, 1.0f);
        //pNormalArray[6] = new Vector3(1.0f,0.0f, 1.0f);
        //pNormalArray[7] = new Vector3(1.0f, 1.0f, 1.0f);
        //pNormalArray[8] = new Vector3(0.0f, 0.0f, 1.0f);
        //pUVArry[6] = new Vector2(0.0f, 1.0f);
        //pUVArry[7] = new Vector2(0.0f, 0.0f);
        //pUVArry[8] = new Vector2(1.0f, 1.0f);
        //pColorArry[6] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[7] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[8] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        //pVector[9] = new Vector3(0.0f, 0.0f, 1.0f);
        //pVector[10] = new Vector3(1.0f, 1.0f, 1.0f);
        //pVector[11] = new Vector3(0.0f, 1.0f, 1.0f);
        //pNormalArray[9] = new Vector3(0.0f, 0.0f, 1.0f);
        //pNormalArray[10] = new Vector3(1.0f, 1.0f, 1.0f);
        //pNormalArray[11] = new Vector3(0.0f, 1.0f, 1.0f);
        //pUVArry[9] = new Vector2(1.0f, 1.0f);
        //pUVArry[10] = new Vector2(0.0f, 0.0f);
        //pUVArry[11] = new Vector2(1.0f, 0.0f);
        //pColorArry[9] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[10] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[11] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        ////left  
        //pVector[12] = new Vector3(0.0f, 0.0f, 1.0f);
        //pVector[13] = new Vector3(0.0f, 1.0f, 1.0f);
        //pVector[14] = new Vector3(0.0f, 0.0f, 0.0f);
        //pNormalArray[12] = new Vector3(0.0f, 0.0f, 1.0f);
        //pNormalArray[13] = new Vector3(0.0f, 1.0f, 1.0f);
        //pNormalArray[14] = new Vector3(0.0f, 0.0f, 0.0f);
        //pUVArry[12] = new Vector2(0.0f, 1.0f);
        //pUVArry[13] = new Vector2(0.0f, 0.0f);
        //pUVArry[14] = new Vector2(1.0f, 1.0f);
        //pColorArry[12] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[13] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[14] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        //pVector[15] = new Vector3(0.0f, 0.0f, 0.0f);
        //pVector[16] = new Vector3(0.0f, 1.0f, 1.0f);
        //pVector[17] = new Vector3(0.0f, 1.0f, 0.0f);
        //pNormalArray[15] = new Vector3(0.0f, 0.0f, 0.0f);
        //pNormalArray[16] = new Vector3(0.0f, 1.0f, 1.0f);
        //pNormalArray[17] = new Vector3(0.0f, 1.0f, 0.0f);
        //pUVArry[15] = new Vector2(1.0f, 1.0f);
        //pUVArry[16] = new Vector2(0.0f, 0.0f);
        //pUVArry[17] = new Vector2(1.0f, 0.0f);
        //pColorArry[15] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[16] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[17] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        ////right  
        //pVector[18] = new Vector3(1.0f, 0.0f, 0.0f);
        //pVector[19] = new Vector3(1.0f, 1.0f, 0.0f);
        //pVector[20] = new Vector3(1.0f, 0.0f, 1.0f);
        //pNormalArray[18] = new Vector3(1.0f, 0.0f, 0.0f);
        //pNormalArray[19] = new Vector3(1.0f, 1.0f, 0.0f);
        //pNormalArray[20] = new Vector3(1.0f, 0.0f, 1.0f);
        //pUVArry[18] = new Vector2(0.0f, 1.0f);
        //pUVArry[19] = new Vector2(0.0f, 0.0f);
        //pUVArry[20] = new Vector2(1.0f, 1.0f);
        //pColorArry[18] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[19] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[20] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        //pVector[21] = new Vector3(1.0f, 0.0f, 1.0f);
        //pVector[22] = new Vector3(1.0f, 1.0f, 0.0f);
        //pVector[23] = new Vector3(1.0f, 1.0f, 1.0f);
        //pNormalArray[21] = new Vector3(1.0f, 0.0f, 1.0f);
        //pNormalArray[22] = new Vector3(1.0f, 1.0f, 0.0f);
        //pNormalArray[23] = new Vector3(1.0f, 1.0f, 1.0f);
        //pUVArry[21] = new Vector2(1.0f, 1.0f);
        //pUVArry[22] = new Vector2(0.0f, 0.0f);
        //pUVArry[23] = new Vector2(1.0f, 0.0f);
        //pColorArry[21] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[22] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[23] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        ////up  
        //pVector[24] = new Vector3(0.0f, 1.0f, 0.0f);
        //pVector[25] = new Vector3(0.0f, 1.0f, 1.0f);
        //pVector[26] = new Vector3(1.0f, 1.0f, 0.0f);
        //pNormalArray[24] = new Vector3(0.0f, 1.0f, 0.0f);
        //pNormalArray[25] = new Vector3(0.0f, 1.0f, 1.0f);
        //pNormalArray[26] = new Vector3(1.0f, 1.0f, 0.0f);
        //pUVArry[24] = new Vector2(0.0f, 1.0f);
        //pUVArry[25] = new Vector2(0.0f, 0.0f);
        //pUVArry[26] = new Vector2(1.0f, 1.0f);
        //pColorArry[24] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[25] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[26] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        //pVector[27] = new Vector3(1.0f, 1.0f, 0.0f);
        //pVector[28] = new Vector3(0.0f, 1.0f, 1.0f);
        //pVector[29] = new Vector3(1.0f, 1.0f, 1.0f);
        //pNormalArray[27] = new Vector3(1.0f, 1.0f, 0.0f);
        //pNormalArray[28] = new Vector3(0.0f, 1.0f, 1.0f);
        //pNormalArray[29] = new Vector3(1.0f, 1.0f, 1.0f);
        //pUVArry[27] = new Vector2(1.0f, 1.0f);
        //pUVArry[28] = new Vector2(0.0f, 0.0f);
        //pUVArry[29] = new Vector2(1.0f, 0.0f);
        //pColorArry[27] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[28] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[29] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        ////down  
        //pVector[30] = new Vector3(0.0f, 0.0f, 0.0f);
        //pVector[31] = new Vector3(1.0f, 0.0f, 0.0f);
        //pVector[32] = new Vector3(1.0f, 0.0f, 1.0f);
        //pNormalArray[30] = new Vector3(0.0f, 0.0f, 0.0f);
        //pNormalArray[31] = new Vector3(1.0f, 0.0f, 0.0f);
        //pNormalArray[32] = new Vector3(1.0f, 0.0f, 1.0f);
        //pUVArry[30] = new Vector2(0.0f, 1.0f);
        //pUVArry[31] = new Vector2(1.0f, 1.0f);
        //pUVArry[32] = new Vector2(1.0f, 0.0f);
        //pColorArry[30] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[31] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[32] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        //pVector[33] = new Vector3(0.0f, 0.0f, 0.0f);
        //pVector[34] = new Vector3(1.0f, 0.0f, 1.0f);
        //pVector[35] = new Vector3(0.0f, 0.0f, 1.0f);
        //pNormalArray[33] = new Vector3(0.0f, 0.0f, 0.0f);
        //pNormalArray[34] = new Vector3(1.0f, 0.0f, 1.0f);
        //pNormalArray[35] = new Vector3(0.0f, 0.0f, 1.0f);
        //pUVArry[33] = new Vector2(0.0f, 1.0f);
        //pUVArry[34] = new Vector2(1.0f, 0.0f);
        //pUVArry[35] = new Vector2(0.0f, 0.0f);
        //pColorArry[33] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[34] = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        //pColorArry[35] = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        //for (int nIndex = 0; nIndex < pTriangles.Length; ++nIndex)
        //{
        //    pTriangles[nIndex] = nIndex;
        //}

        //m_mesh.Clear();
        //m_mesh.vertices = pVector;//网格顶点  
        //m_mesh.normals = pNormalArray;//法线
        //m_mesh.uv = pUVArry;//UV
        //m_mesh.colors = pColorArry;//Color
        //m_mesh.triangles = pTriangles;//三角形  
        //m_mesh.RecalculateBounds();
        ////创建纹理材质
        //Material mat = new Material(Shader.Find("Mobile/VertexLit"));
        //Texture item_Info_Txt = Resources.Load("MAP", typeof(Texture)) as Texture;
        //mat.mainTexture = item_Info_Txt;


        //gameObject.GetComponent<MeshRenderer>().material = mat;
    }


    void Update()
    {
        //gameObject.transform.Rotate(new Vector3(0, m_rotationY + Time.deltaTime * m_speed, 0));
        //_mesh.Clear();
        //_mesh.vertices = pVector;//网格顶点  
        //_mesh.triangles = pTriangles;//三角形  
        //_mesh.RecalculateBounds();
        ////点击鼠标左键  
        //if (Input.GetMouseButton(0))
        //{
        //    //顶点数+1  
        //    count++;
        //    //将获得的鼠标坐标转换为世界坐标，然后添加到list链表中。  
        //    list.Add(Camera.main.ScreenToWorldPoint(new Vector3(Input.mousePosition.x, Input.mousePosition.y, 0.8f)));

        //}

        ////如果顶点数>=3，那么就开始渲染Mesh  
        //if (count >= 3)
        //{
        //    //根据顶点数来计算绘制出三角形的所以顶点数  
        //    triangles = new int[3 * (count - 2)];
        //    //根据顶点数来创建记录顶点坐标  
        //    vertices = new Vector3[count];
        //    //将链表中的顶点坐标赋值给vertices  
        //    for (int i = 0; i < count; i++)
        //    {
        //        vertices[i] = list[i];

        //    }

        //    //三角形个数  
        //    int triangles_count = count - 2;
        //    //根据三角形的个数，来计算绘制三角形的顶点顺序（索引）  
        //    for (int i = 0; i < triangles_count; i++)
        //    {
        //        //这个算法好好琢磨一下吧~  
        //        triangles[3 * i] = 0;
        //        triangles[3 * i + 1] = i + 2;
        //        triangles[3 * i + 2] = i + 1;
        //    }
        //    //设置顶点坐标  
        //    mesh.vertices = vertices;
        //    //设置顶点索引  
        //    mesh.triangles = triangles;
        //}

    }

    //Material createMeshMaterail(string p)
    //{
    //    Material mater = new Material(Shader.Find("Transparent/Diffuse"));
    //    return mater;
    //}
    public int[] GetRangeArray(int[] SourceArray, int StartIndex, int EndIndex)
    {
           int[] result = new int[EndIndex - StartIndex + 1];
           for (int i = 0; i <= EndIndex - StartIndex; i++) 
               result[i] = SourceArray[i + StartIndex];
            return result;
     }
    IEnumerator IGetSKPTexture(string filePath,int materialIndex)
    {
        WWW www = new WWW("file:///C:/Users/Administrator/Desktop/uv_test1cube/Wood_Floor_Dark.jpg");
        yield return www.isDone;
        this.gameObject.GetComponent<MeshRenderer>().materials[materialIndex].mainTexture = www.texture;
    }
}
