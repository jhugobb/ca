#include "cal3d.h"
#include <iostream>

int main() {

  CalCoreModel* model = new CalCoreModel("dummy");
  model->loadCoreSkeleton("/home/yellow/Desktop/MIRI/2year/ca/Crowd/skeleton/skeleton.csf");
  
  int idleAnimationId;

  idleAnimationId = model->loadCoreAnimation("/home/yellow/Desktop/MIRI/2year/ca/Crowd/skeleton/skeleton_idle.caf");
  // idleAnimationId = model->loadCoreAnimation("/home/yellow/Desktop/MIRI/2year/ca/Crowd/skeleton/skeleton_jog.caf");

  int chestMeshId;

  chestMeshId = model->loadCoreMesh("/home/yellow/Desktop/MIRI/2year/ca/Crowd/skeleton/skeleton_chest.cmf");
  CalModel* normalModel;
  normalModel = new CalModel(model);

  if (!normalModel->attachMesh(chestMeshId)){
    std::cout << "fuck?" << std::endl;
  }
  return 0;
}