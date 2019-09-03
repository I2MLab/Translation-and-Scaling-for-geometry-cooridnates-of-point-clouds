// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm> 
#include <io.h>
#include <cctype> 

using namespace std;

bool compareNat(const std::string& a, const std::string& b)//Read the file from the folder based on the number in the file name
{
	if (a.empty())
		return true;
	if (b.empty())
		return false;
	if (std::isdigit(a[0]) && !std::isdigit(b[0]))
		return true;
	if (!std::isdigit(a[0]) && std::isdigit(b[0]))
		return false;
	if (!std::isdigit(a[0]) && !std::isdigit(b[0]))
	{
		if (std::toupper(a[0]) == std::toupper(b[0]))
			return compareNat(a.substr(1), b.substr(1));
		return (std::toupper(a[0]) < std::toupper(b[0]));
	}

	// Both strings begin with digit --> parse both numbers
	std::istringstream issa(a);
	std::istringstream issb(b);
	int ia, ib;
	issa >> ia;
	issb >> ib;
	if (ia != ib)
		return ia < ib;

	// Numbers are the same --> remove numbers and recurse
	std::string anew, bnew;
	std::getline(issa, anew);
	std::getline(issb, bnew);
	return (compareNat(anew, bnew));
}

void GetAllFormatFiles(string path, vector<string>& files, string format)//find all the ply files in the specific folder
{
	long   hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*" + format).c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );  
					GetAllFormatFiles(p.assign(path).append("\\").append(fileinfo.name), files, format);
				}
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
	std::sort(files.begin(), files.end(), compareNat);
}

void GetScaler(string path, int Scaler[3])//Get the Scaler from the bounding box of the point cloud
{
	ifstream fin(path);
	if (fin.is_open())
	{
		string a;
		float maxX=0, minX=0;//minmum and maximu geometry of X direction
		float maxY=0, minY=0;
		float maxZ=0, minZ=0;
		while (getline(fin, a))
		{
			int i = 0;
			string tempg;
			istringstream a_str;//which is separated by space
			float g[7];
			a_str.str(a);
			while (a_str >> tempg)
			{
				g[i] = atof(tempg.c_str());
				i++;
			}

			for (int d = 0;d <3;d++)
			{

					if (g[0] > maxX)
					{
						maxX = g[0];
					}
					if (g[0] <= minX)
					{
						minX = g[0];
					}
					if (g[1] > maxY)
					{
						maxY = g[1];
					}
					if (g[1] <= minY)
					{
						minY = g[1];
					}
					if (g[2] > maxZ)
					{
						maxZ = g[2];
					}
					if (g[2] <= minZ)
					{
						minZ = g[2];
					}
	
			}
			
		}
		Scaler[0] = (1023) / (maxX - minX);//1023 means the bounding box is 2^10， and there is 10 levels, this can be changed to others as well
		Scaler[1]= (1023) / (maxY - minY);//1023 means the bounding box is 2^10， and there is 10 levels, this can be changed to others as well
		Scaler[2] = (1023) / (maxZ - minZ);//1023 means the bounding box is 2^10， and there is 10 levels, this can be changed to others as well
	}

}

int main()
{
	int min, max;
	min = 0;
	max = 0;
	string filePath1 = "D:\\SelectedMeshes\\AxeGuy_ply_400K_trans_ASC\\";//the input point cloud folder
	vector<string> files1;
	string format = ".ply";
	GetAllFormatFiles(filePath1, files1, format);
	int size = files1.size();
	string s;
	string a, b;
	int d;
	for (int i = 0;i<size;i++)
	{
	cout << files1[i] << endl;
	ifstream fin(files1[i]);
	string files2 = "D:\\SelectedMeshes\\AxeGuy_ply_400K_final2\\AxeGuy_ply_400K_" + to_string(i) + format;//the output point cloud
	ofstream fout(files2);

	if (fin.is_open())
	{
		while (getline(fin, a))
		{
			if ((a!="format ascii 1.0") &&(a.find('.') != string::npos))
			{
				int i = 0;
				string tempg;
				istringstream a_str;//which is separated by space
				float g[7];
				int g_int[6];
				vector<string> o_str;
				a_str.str(a);
				while (a_str >> tempg)
				{
					g[i] = atof(tempg.c_str());
					i++;
					
				}
				
				for (int d = 0;d <6;d++)
				{
					if (d < 3)
					{
						g_int[d] = (int) (g[d] * 1000);
						//scale the position of the point depdending on the scaling factor, this scaling factor depends on the bounding box of the point cloud
						//1000 can be obtained from the function of GetScaler() in general, here for simplicty the scaler is set to 1000 for x,y,z directions.
					}
					else
					{ 
						g_int[d] = (int)(g[d]);//the color information is kept as the same
					}			

					o_str.push_back(to_string(g_int[d]));
				}
				for (int j = 0;j < 6;++j)
				{
					fout << o_str[j]<<" ";
			   }
				fout << "\n";
				
			}

			
			else
			{
				if((a!="property uchar alpha")&& (a.find("element face") == string::npos)&&(a!="property list uchar int vertex_indices"))
				{ 
				if (a[0] != '3')
				{
					if (a[1] != ' ')
					{
						fout << a;
						fout << "\n";

					}

				}
				
				}
			}

			/*
			if (s.find('.') != string::npos)
			{
			b = std::atof(s.c_str()) / 2;
			a = to_string(b);
			}
			fout << a;
			*/
		}
	}
	}
	cout << "The process of conversion to integer positions is completed" << endl;
}


