#include "eus_pcl/euspcl.h"

#include <octomap/octomap.h>

#if __PCL_SELECT == 0
using namespace pcl;
#elif __PCL_SELECT == 17
using namespace pcl17;
#endif

pointer OCTOMAP_CREATE (register context *ctx, int n, pointer *argv) {
  /* resolution (data-string) (full) -> octree_pointer */
  numunion nu;
  double resolution = 0.05;
  ckarg2(0, 2);
  if (n > 0) {
    resolution = ckfltval(argv[0]) * 0.001;
  }
  octomap::OcTree *tree_ptr;
  tree_ptr = new octomap::OcTree ( resolution );

  if (n > 1 && isstring(argv[1])) {
    std::stringstream datastream;
    long len = strlength(argv[1]);
    datastream.write((const char *)(argv[1]->c.str.chars), len);
    if (n > 2 && ( argv[2] != NIL )) {
      // use full data
      tree_ptr->readBinaryData(datastream);
    } else {
      // use binary data
      tree_ptr->readBinaryData(datastream);
    }
    return makeint((eusinteger_t)tree_ptr);
  }

  return makeint((eusinteger_t)tree_ptr);
}

pointer OCTOMAP_DELETE (register context *ctx, int n, pointer *argv) {
  /* octree_pointer -> nil */
  numunion nu;
  void *tree_ptr;

  ckarg(1);
  tree_ptr = (void *)(ckintval(argv[0]));

  delete ( (octomap::OcTree *)tree_ptr );

  return NIL;
}

pointer OCTOMAP_RESOLUTION (register context *ctx, int n, pointer *argv) {
  /* octree_pointer (resolution) -> resolution */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg2(1, 2);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  if (n > 1) {
    if (argv[0] != NIL) {
      double r = ckfltval(argv[0]) * 0.001;
      tree_ptr->setResolution( r );
    }
  }

  eusfloat_t resolution;
  resolution = tree_ptr->getResolution() * 1000.0;

  return makeflt(resolution);
}

pointer OCTOMAP_BOUNDING_BOX (register context *ctx, int n, pointer *argv) {
  /* octree_pointer (min_vec) (max_vec) -> (min_vec . max_vec) */
  numunion nu;
  octomap::OcTree *tree_ptr;
  octomap::point3d pmin, pmax;
  bool set_value = false;
  ckarg2(1, 3);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  pmin = tree_ptr->getBBXMin();
  pmax = tree_ptr->getBBXMax();

  if (n > 1) {
    if (isfltvector(argv[1])) {
      pointer min_vec = argv[1];
      pmin.x() = min_vec->c.fvec.fv[0] * 0.001;
      pmin.y() = min_vec->c.fvec.fv[1] * 0.001;
      pmin.z() = min_vec->c.fvec.fv[2] * 0.001;
      set_value = true;
    }
  }
  if (n > 2) {
    if (isfltvector(argv[2])) {
      pointer max_vec = argv[2];
      pmax.x() = max_vec->c.fvec.fv[0] * 0.001;
      pmax.y() = max_vec->c.fvec.fv[1] * 0.001;
      pmax.z() = max_vec->c.fvec.fv[2] * 0.001;
      set_value = true;
    }
  }

  if (set_value) {
    tree_ptr->setBBXMax(pmax);
    tree_ptr->setBBXMin(pmin);
  }

  pointer ret_min = makefvector(3);
  vpush(ret_min);
  ret_min->c.fvec.fv[0] = pmin.x() * 1000.0;
  ret_min->c.fvec.fv[1] = pmin.y() * 1000.0;
  ret_min->c.fvec.fv[2] = pmin.z() * 1000.0;

  pointer ret_max = makefvector(3);
  vpush(ret_max);
  ret_max->c.fvec.fv[0] = pmax.x() * 1000.0;
  ret_max->c.fvec.fv[1] = pmax.y() * 1000.0;
  ret_max->c.fvec.fv[2] = pmax.z() * 1000.0;

  pointer ret = rawcons(ctx, ret_min, ret_max);
  vpop(); vpop();
  return ret;
}

pointer OCTOMAP_BOUNDING_BOX_INFO (register context *ctx, int n, pointer *argv) {
  /* octree_pointer -> (bounding-box-bounds . bounding-box-center) */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg(1);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  octomap::point3d bbxb = tree_ptr->getBBXBounds();
  octomap::point3d bbxc = tree_ptr->getBBXCenter();

  pointer bbx_bd = makefvector(3);
  vpush(bbx_bd);
  bbx_bd->c.fvec.fv[0] = bbxb.x() * 1000.0;
  bbx_bd->c.fvec.fv[1] = bbxb.y() * 1000.0;
  bbx_bd->c.fvec.fv[2] = bbxb.z() * 1000.0;

  pointer bbx_ct = makefvector(3);
  vpush(bbx_ct);
  bbx_ct->c.fvec.fv[0] = bbxc.x() * 1000.0;
  bbx_ct->c.fvec.fv[1] = bbxc.y() * 1000.0;
  bbx_ct->c.fvec.fv[2] = bbxc.z() * 1000.0;

  pointer ret = rawcons(ctx, bbx_bd, bbx_ct);
  vpop(); vpop();
  return ret;
}

pointer OCTOMAP_CLAMPING_THRESHOLD (register context *ctx, int n, pointer *argv) {
  /* octree_pointer (clamping-min) (clamping-max) -> (clamping-min . clamping-max) */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg2(1, 3);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  double min_threshold = 0.1192; /* check default */
  double max_threshold = 0.971; /* check default */

  if (n > 1) {
    min_threshold = ckfltval(argv[1]);
    tree_ptr->setClampingThresMin(min_threshold);
  }
  if (n > 2) {
    max_threshold = ckfltval(argv[2]);
    tree_ptr->setClampingThresMax(max_threshold);
  }

  min_threshold = tree_ptr->getClampingThresMin();
  max_threshold = tree_ptr->getClampingThresMax();
#if 0
float getClampingThresMinLog () const
float getClampingThresMaxLog () const
#endif
  pointer mint, maxt;
  mint = makeflt(min_threshold);
  vpush(mint);
  maxt = makeflt(max_threshold);
  vpush(maxt);
  pointer ret = rawcons(ctx, mint, maxt);
  vpop(); vpop();

  return ret;
}

pointer OCTOMAP_METRIC_INFO (register context *ctx, int n, pointer *argv) {
  /* octree_pointer -> (metric-min metrix-max metric-size) */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg(1);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  double max_x, max_y, max_z;
  double min_x, min_y, min_z;
  double size_x, size_y, size_z;
  tree_ptr->getMetricMin (min_x, min_y, min_z);
  tree_ptr->getMetricMax (max_x, max_y, max_z);
  tree_ptr->getMetricSize (size_x, size_y, size_z);

  pointer min_vec = makefvector(3);
  vpush(min_vec);
  min_vec->c.fvec.fv[0] = min_x * 1000.0;
  min_vec->c.fvec.fv[1] = min_y * 1000.0;
  min_vec->c.fvec.fv[2] = min_z * 1000.0;

  pointer max_vec = makefvector(3);
  vpush(max_vec);
  max_vec->c.fvec.fv[0] = max_x * 1000.0;
  max_vec->c.fvec.fv[1] = max_y * 1000.0;
  max_vec->c.fvec.fv[2] = max_z * 1000.0;

  pointer size_vec = makefvector(3);
  vpush(size_vec);
  size_vec->c.fvec.fv[0] = size_x * 1000.0;
  size_vec->c.fvec.fv[1] = size_y * 1000.0;
  size_vec->c.fvec.fv[2] = size_z * 1000.0;

  pointer ret = rawcons(ctx, size_vec, NIL);
  vpush(ret);
  ret = rawcons(ctx, max_vec, ret);
  vpush(ret);
  ret = rawcons(ctx, min_vec, ret);
  vpop(); vpop(); vpop();
  vpop(); vpop();

  return ret;
}

pointer OCTOMAP_OCCUPANCY_THRESHOLD (register context *ctx, int n, pointer *argv) {
  /* octomap_pointer (occupancy-threshold) -> occupancy-threshold */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg2(1,2);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  if (n > 1 && (argv[1] != NIL)) {
    double val = ckfltval(argv[1]);
    tree_ptr->setOccupancyThres(val);
  }

  double dval = tree_ptr->getOccupancyThres();
#if 0
  float getOccupancyThresLog () const
#endif
  return makeflt(dval);
}

pointer OCTOMAP_PROBABILITY (register context *ctx, int n, pointer *argv) {
  /* octomap_pointer (probability-hit) (probability-miss) -> (probability-hit . probability-miss) */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg2(1, 3);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  double prob_hit  = 0.7; /* default value */
  double prob_miss = 0.4; /* default value */
  if (n > 1 && (argv[1] != NIL)) {
    prob_hit = ckfltval(argv[1]);
    tree_ptr->setProbHit (prob_hit);
    // float getProbHitLog () const
  }
  if (n > 2 && (argv[2] != NIL)) {
    prob_miss = ckfltval(argv[2]);
    tree_ptr->setProbMiss (prob_miss);
    // float getProbMissLog () const
  }

  prob_hit  = tree_ptr->getProbHit();
  prob_miss = tree_ptr->getProbMiss();

  pointer hit_eus, miss_eus;
  hit_eus  = makeflt(prob_hit);
  vpush(hit_eus);
  miss_eus = makeflt(prob_miss);
  vpush(miss_eus);
  pointer ret = rawcons(ctx, hit_eus, miss_eus);
  vpop(); vpop();

  return ret;
}

pointer OCTOMAP_GET_TREE_INFO (register context *ctx, int n, pointer *argv) {
  /* octomap_pointer -> (depth . type-string) */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg(1);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));
  eusinteger_t depth = tree_ptr->getTreeDepth();
  pointer eus_depth = makeint(depth);
  vpush(eus_depth);
  std::string type_str = tree_ptr->getTreeType();
  pointer eus_str = makestring((char *)type_str.c_str(), type_str.length());
  vpush(eus_str);

  pointer ret = rawcons (ctx, eus_depth, eus_str);
  vpop(); vpop();

  return ret;
}

pointer OCTOMAP_USE_BBX_LIMIT (register context *ctx, int n, pointer *argv) {
  /* octomap_pointer enable */
  numunion nu;
  octomap::OcTree *tree_ptr;
  pointer ret = NIL;
  ckarg(2);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  bool enable = false;
  if (argv[1] != NIL) {
    ret = T;
    enable = true;
  }
  tree_ptr->useBBXLimit (enable);

  return ret;
}

pointer OCTOMAP_NODE_NUM (register context *ctx, int n, pointer *argv) {
  /* octree_pointer -> number_of_nodes */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg(1);
  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  eusinteger_t ret = tree_ptr->calcNumNodes();
  //size_t getNumLeafNodes () const

  return makeint(ret);
}

pointer OCTOMAP_READ_NODES (register context *ctx, int n, pointer *argv) {
  /* octree_pointer (depth) -> (occupied_points freer_points) */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg2(1, 2);
  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  int depth = 0;
  if (n > 1) {
    depth = ckintval(argv[1]);
  }

  PointCloud< Point > occ_points;
  PointCloud< Point > free_points;
  for (octomap::OcTree::iterator it = tree_ptr->begin(depth), end = tree_ptr->end();
       it != end; ++it) {
    if (tree_ptr->isNodeOccupied(*it)) { // occupied
      Point p(it.getX(),
              it.getY(),
              it.getZ());
      occ_points.push_back(p);
    } else { // free
      Point p(it.getX(),
              it.getY(),
              it.getZ());
      free_points.push_back(p);
    }
  }

  pointer ret = NIL;
  {
    pointer ret_free = make_pointcloud_from_pcl (ctx, free_points);
    vpush(ret_free);
    ret = rawcons (ctx, ret_free, ret);
    vpop(); vpush(ret);
    pointer ret_occ = make_pointcloud_from_pcl (ctx, occ_points);
    vpush(ret_occ);
    ret = rawcons (ctx, ret_occ, ret);
    vpop(); vpop();
  }

  return ret;
}

pointer OCTOMAP_READ_UNKNOWN (register context *ctx, int n, pointer *argv) {
  /* octree_pointer (depth) (min_vec) (max_vec) -> unknown_points */
  numunion nu;
  octomap::OcTree *tree_ptr;

  ckarg2(1,4);
  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  int depth = 0;
  if (n > 1) {
    depth = ckintval(argv[1]);
  }

  octomap::point3d pmin, pmax;
  pmax = tree_ptr->getBBXMax();
  pmin = tree_ptr->getBBXMin();

  if (n > 2 && isfltvector(argv[2])) {
    pointer min_vec = argv[2];
    pmin.x() = min_vec->c.fvec.fv[0] * 0.001;
    pmin.y() = min_vec->c.fvec.fv[1] * 0.001;
    pmin.z() = min_vec->c.fvec.fv[2] * 0.001;
  }
  if (n > 3 && isfltvector(argv[3])) {
    pointer max_vec = argv[3];
    pmax.x() = max_vec->c.fvec.fv[0] * 0.001;
    pmax.y() = max_vec->c.fvec.fv[1] * 0.001;
    pmax.z() = max_vec->c.fvec.fv[2] * 0.001;
  }

  //tree_ptr->getUnknownLeafCenters (node_centers, pmin, pmax);
  //// unknown leafs
  int tree_depth = tree_ptr->getTreeDepth();
  if (depth == 0)
    depth = tree_depth;
  double step_size =  tree_ptr->getResolution() * pow(2, tree_depth-depth);
  //printf("%d %f\n", tree_depth, step_size);

  int start_idx[3];
  int end_idx[3];
  for (int i = 0; i < 3; i++) {
    end_idx[i] = floor((pmax(i)+step_size/2)/step_size);
    start_idx[i] = floor((pmin(i)+step_size/2)/step_size) + 1;
    //printf("%d / %d %d %f %f\n",i,start_idx[i], end_idx[i], pmax(i), pmin(i));
  }

  PointCloud< Point > pc;
  octomap::point3d pt;
  octomap::OcTree::NodeType* res;
  for (int x = start_idx[0]; x <= end_idx[0]; x++) {
    pt.x() = (x - 0.5) * step_size;
    for (int y = start_idx[1]; y <= end_idx[1]; y++) {
      pt.y() = (y - 0.5) * step_size;
      for (int z = start_idx[2]; z <= end_idx[2]; z++) {
        pt.z() = (z - 0.5) * step_size;
        res = tree_ptr->search(pt, depth);
          if (res == NULL) {
          Point p(pt.x(), pt.y(), pt.z());
          pc.push_back(p);
        }
      }
    }
  }
  //// end_of unknown leafs

  return make_pointcloud_from_pcl (ctx, pc);
}

pointer OCTOMAP_ADD_POINTS (register context *ctx, int n, pointer *argv) {
  /* octree_pointer points origin -> number_of_nodes */
  numunion nu;
  pointer pcloud;
  octomap::OcTree *tree_ptr;

  ckarg(3);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  if (!isPointCloud (argv[1])) {
    error(E_TYPEMISMATCH);
  }
  pcloud = argv[1];
  int width = intval (get_from_pointcloud (ctx, pcloud, K_EUSPCL_WIDTH));
  int height = intval (get_from_pointcloud (ctx, pcloud, K_EUSPCL_HEIGHT));
  pointer points = get_from_pointcloud (ctx, pcloud, K_EUSPCL_POINTS);

  PointCloud< Point >::Ptr pcl_cloud =
    make_pcl_pointcloud< Point > (ctx, points, NULL, NULL, NULL, width, height);

  pointer e_origin = argv[2];
  octomap::point3d origin(e_origin->c.fvec.fv[0] * 0.001,
                          e_origin->c.fvec.fv[1] * 0.001,
                          e_origin->c.fvec.fv[2] * 0.001);

  octomap::Pointcloud pt;
  for (PointCloud< Point >::const_iterator it = pcl_cloud->begin(); it != pcl_cloud->end(); ++it) {
    pt.push_back(it->x, it->y, it->z);
  }

  tree_ptr->insertPointCloud (pt, origin);//
  //// finish insert

  eusinteger_t ret = tree_ptr->calcNumNodes();
  return makeint(ret);
}

pointer OCTOMAP_DUMP_DATA (register context *ctx, int n, pointer *argv) {
  /* octomap_pointer (binary) -> string */
  octomap::OcTree *tree_ptr;
  std::stringstream datastream;
  bool binary = false;

  ckarg2(1, 2);

  tree_ptr = (octomap::OcTree *)(ckintval(argv[0]));

  if (n > 1) {
    if (argv[1] != NIL) {
      binary = true;
    }
  }
  if (binary) {
    if (!tree_ptr->writeBinaryData(datastream)) {
      return NIL;
    }
  } else {
    if (!tree_ptr->write(datastream)) {
      return NIL;
    }
  }

  return makestring((char *)datastream.str().c_str(), datastream.str().length());
}