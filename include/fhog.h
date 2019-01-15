/*
 * fhog.h
 *
 *  Created on: 2016-11-13
 *      Author: ZQ
 */

#ifndef FHOG_H_
#define FHOG_H_


int allocFeatureMapObject(CvLSVMFeatureMapCaskade **obj, const int sizeX,
                          const int sizeY, const int numFeatures);
int freeFeatureMapObject (CvLSVMFeatureMapCaskade **obj);
int getFeatureMaps(const IplImage* image, const int k, CvLSVMFeatureMapCaskade **map);
int normalizeAndTruncate(CvLSVMFeatureMapCaskade *map, const float alfa);
int PCAFeatureMaps(CvLSVMFeatureMapCaskade *map);

#endif /* FHOG_H_ */
