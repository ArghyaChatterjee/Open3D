// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include <iostream>
#include <memory>

#include "open3d/Open3D.h"

using namespace open3d;

int main(int argc, char *argv[]) {
    // Argument 1: Device: 'CPU:0' for CPU, 'CUDA:0' for GPU
    // Argument 2: Path to Source PointCloud
    // Argument 3: Path to Target PointCloud

    // TODO: Take this input as arguments
    auto device = core::Device(argv[1]);
    auto dtype = core::Dtype::Float32;

    // t::io::ReadPointCloud, changes the device to CPU and DType to Float64
    t::geometry::PointCloud source_, target_;
    // t::geometry::PointCloud target(device);
    t::io::ReadPointCloud(argv[2], source_, {"auto", false, false, true});
    t::io::ReadPointCloud(argv[3], target_, {"auto", false, false, true});
    // t::geometry::PointCloud target = t::io::
    // geometry::PointCloud legacy = source_.ToLegacyPointCloud();

    // legacy.VoxelDownSample(25.0);
    // legacy.EstimateNormals(open3d::geometry::KDTreeSearchParamKNN(), false);
    // t::geometry::PointCloud source =
    //         t::geometry::PointCloud::FromLegacyPointCloud(legacy);

    // t::geometry::PointCloud target =
    //         t::geometry::PointCloud::FromLegacyPointCloud(legacy);
    // Creating Tensor from manual transformation vector
    core::Tensor trans =
            core::Tensor::Init<float>({{0.862, 0.011, -0.507, 0.5},
                                       {-0.139, 0.967, -0.215, 0.7},
                                       {0.487, 0.255, 0.835, -1.4},
                                       {0.0, 0.0, 0.0, 1.0}},
                                      device);
    // target = target.Transform(trans);
    core::Tensor source_points =
            source_.GetPoints().To(device, dtype, /*copy=*/true);
    t::geometry::PointCloud source_device(device);
    source_device.SetPoints(source_points);
    core::Tensor target_points =
            target_.GetPoints().To(device, dtype, /*copy=*/true);
    core::Tensor target_normals =
            target_.GetPointNormals().To(device, dtype, /*copy=*/true);
    t::geometry::PointCloud target_device(device);
    target_device.SetPoints(target_points);
    target_device.SetPointNormals(target_normals);

    // core::Tensor init_trans = core::Tensor::Eye(4, dtype, device);

    utility::LogInfo(" Input on {} Success", device.ToString());
    double max_correspondence_dist = 0.02;

    t::pipelines::registration::RegistrationResult evaluation(trans);
    // for (int i = 0; i < itr; i++) {
    //     eval_timer.Start();
    evaluation = open3d::t::pipelines::registration::EvaluateRegistration(
            source_device, target_device, max_correspondence_dist, trans);

    // ICP ConvergenceCriteria for both Point To Point and Point To Plane:
    double relative_fitness = 1e-6;
    double relative_rmse = 1e-6;
    int max_iterations = 5;

    // // ICP: Point to Plane
    // utility::Timer icp_p2plane_time;
    // icp_p2plane_time.Start();
    auto reg_p2plane = open3d::t::pipelines::registration::RegistrationICP(
            source_device, target_device, max_correspondence_dist, trans,
            open3d::t::pipelines::registration::
                    TransformationEstimationPointToPoint()
            /*open3d::t::pipelines::registration::ICPConvergenceCriteria(
                    relative_fitness, relative_rmse, max_iterations)*/);
    // icp_p2plane_time.Stop();
    // Printing result for ICP Point to Plane
    utility::LogInfo(" [ICP: Point to Plane] ");
    utility::LogInfo("   Convergence Criteria: ");
    utility::LogInfo(
            "   Relative Fitness: {}, Relative Fitness: {}, Max Iterations {}",
            relative_fitness, relative_rmse, max_iterations);
    utility::LogInfo("   EvaluateRegistration on {} Success ",
                     device.ToString());
    utility::LogInfo("     [PointCloud Size]: ");
    utility::LogInfo("       Points: {} Target Points: {} ",
                     source_points.GetShape().ToString(),
                     target_points.GetShape().ToString());
    utility::LogInfo("       Fitness: {} ", reg_p2plane.fitness_);
    utility::LogInfo("       Inlier RMSE: {} ", reg_p2plane.inlier_rmse_);
    // utility::LogInfo("     [Time]: {}", icp_p2plane_time.GetDuration());
    utility::LogInfo("     [Transformation Matrix]: \n{}",
                     reg_p2plane.transformation_.ToString());

    // auto transformation_point2plane = reg_p2plane.transformation_;
    // VisualizeRegistration(source2, target, transformation_point2plane);
    return 0;
}