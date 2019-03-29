/** @file:     potential_landscape.h
 *  @author:   Samuel
 *  @created:  2019.03.17
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Stores potential landscape.
 */

#ifndef _COMP_POT_LANDSCAPE_H_
#define _COMP_POT_LANDSCAPE_H_

#include <QtWidgets>

#include "job_result.h"

namespace comp{

  //! Stores electron configurations of DB layouts.
  class PotentialLandscape : public JobResult
  {
    Q_OBJECT

  public:

    //! Empty constructor.
    PotentialLandscape() : JobResult(PotentialLandscapeResult) {};

    //! Constructor taking a QXmlStreamReader to read the results directly.
    PotentialLandscape(QXmlStreamReader *rs, const QString &result_dir_path);

    // TODO alternative constructor taking relevant information
    
    //! Destructor.
    ~PotentialLandscape() {};

    //! Return a list of vector of potentials (TODO change to struct).
    //! TODO add z-height specification
    QList<QVector<float>> potentials() const {return potential_vals;}

    // TODO in the future, store 3D result and let users choose which slice to show

    //! Return the path to the static image plot.
    QString staticPlotPath() {return static_plot_path;}

    //! Return the path to the animation.
    QString animationPath() {return animation_path;}

    //! Return the path to the plot legend.
    QString plotLegendPath() {return plot_legend_path;}


  private:

    QList<QVector<float>> potential_vals; //!< potentials[result_ind][0] is x, ...[1] is y, ...[2] is potential value

    QString static_plot_path;     //!< Path to static 2D slice plot
    QString animation_path;       //!< Path to 2D slice potential animation gif
    QString plot_legend_path;     //!< Path to static slice plot with legend

  };

} // end of comp namespace

#endif
