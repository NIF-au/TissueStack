var webpack = require('webpack');
var path = require("path");

var CommonsChunkPlugin = require("./node_modules/webpack/lib/optimize/CommonsChunkPlugin");

/** a small routine to purge the build dir contents */
var fs = require('fs');
var deleteFolderRecursive = function(path) {
  if( fs.existsSync(path) ) {
    fs.readdirSync(path).forEach(function(file,index){
      var curPath = path + "/" + file;
      if(fs.lstatSync(curPath).isDirectory()) { // recurse
        deleteFolderRecursive(curPath);
      } else { // delete file
        fs.unlinkSync(curPath);
      }
    });
    fs.rmdirSync(path);
  }
};
deleteFolderRecursive('./build')

module.exports = {
    entry : {
        tissuestack: './src/js/main'
    },
    output : {
        path: path.join(__dirname, "build"),
        filename : "[name].js",
        library : ['tissuestack'],
        libraryTarget : "var"
        //target : "web"
    },
    //externals: {},
    plugins: [
        new CommonsChunkPlugin({
            name: "tilecache",
            chunks: ["tilecache"]
        })
    ],
    module: {
        loaders: [
            { test: /\.js$/, loader: 'babel',
                exclude: '/node_modules', query : {presets: ['es2015', 'stage-1'] }}
        ]
    }
};
