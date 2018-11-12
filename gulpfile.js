const gulp = require('gulp');
const del = require('del');
const uglify = require('gulp-uglify');
const inline = require('gulp-inline');
const cleancss = require('gulp-clean-css');
const htmlmin = require('gulp-htmlmin');
const gzip = require('gulp-gzip');
const fs = require('fs');


gulp.task('copiar', function () {
    return gulp.src([
        "pagina/*.html"
    ])
        .pipe(
            gulp.dest("server/")
        );
});

gulp.task('inline',  function () {
    return gulp.src('pagina/*.html').pipe(
        inline({
                base: 'pagina/',
                js: uglify,
                css: cleancss
            }
        )
    ).pipe(htmlmin({
        collapseWhitespace: true,
        removeComments: true,
        minifyCSS: true,
        minifyJS: true
    })).pipe(gzip()
    ).pipe(
        gulp.dest("build/")
    )
});

gulp.task('generarh', ['inline'], function () {
    let source = "build/index.html.gz";
    let destino = "server/index.html.gz.h";

    let wstream = fs.createWriteStream(destino);

    wstream.on('error', (err) => {
        console.log(err);
    });

    let data = fs.readFileSync(source);
    wstream.write("#define index_html_gz_len " + data.length + "\n");

    wstream.write("const char index_html_gz[] = {\n");

    for (let i = 0; i < data.length; i++) {
        if (i % 30 == 0)
            wstream.write('\n');

        let d = ('00' + data[i].toString(16)).slice(-2);

        wstream.write("0x" + d);
        if (i < data.length - 1)
            wstream.write(', ');
    }
    wstream.write('\n};');
    wstream.end();


});

// Para agregar despues (Final)

/*
	Genera el .h de la pagina de carga exitosa.
*/

gulp.task('generarh2', ['inline'], function () {
    let source = "build/config.html.gz";
    let destino = "server/config.html.gz.h";

    let wstream = fs.createWriteStream(destino);

    wstream.on('error', (err) => {
        console.log(err);
    });

    let data = fs.readFileSync(source);
    wstream.write("#define config_html_gz_len " + data.length + "\n");

    wstream.write("const char config_html_gz[] = {\n");

    for (let i = 0; i < data.length; i++) {
        if (i % 30 == 0)
            wstream.write('\n');

        let d = ('00' + data[i].toString(16)).slice(-2);

        wstream.write("0x" + d);
        if (i < data.length - 1)
            wstream.write(', ');
    }
    wstream.write('\n};');
    wstream.end();


});

gulp.task('default', ['generarh']);

gulp.task('default2', ['generarh2']); // Genero el archivo de config.html

//gulp.task('generar')
