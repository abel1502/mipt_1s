#ifndef LATEX_CONSTS_H
#define LATEX_CONSTS_H

namespace SymbolDiff {

    struct LatexConsts {
        static constexpr const char header[] =
            "\\documentclass[12pt]{article}\n"
            "\\usepackage[utf8]{inputenc}\n"
            "\\usepackage{textcomp}\n"
            "\\usepackage{scalerel}\n"
            "\\usepackage{newunicodechar}\n"
            "\\usepackage{graphicx}\n"
            "\\usepackage{indentfirst}\n"
            "\\newunicodechar{\u2642}{\\scalerel*{\\includegraphics{exprs/u2642.png}}{\\textrm{" "\\#" /*"\\textbigcircle"*/ "}}}\n\n"
            "\\begin{document}\n\n";

        static constexpr const char titlePage[] =
            "\\begin{titlepage}"
            "\\begin{center}\\Large MIPT ICT, 2020\\end{center}"
            "\\begin{center}\\Large\\textit{Belyaev Andrey Alexeevich}\\end{center}"
            "\\begin{center}\\Huge\\textbf{\\underline{D I F F E R E N T I A T I O N}}\\end{center}"
            "\\begin{center}\\large\\textit{Patent pending}\\end{center}"
            "\\section{\\Large{Abstract}}\n"
            "So, scientific works get paid for, right? I've got a master plan..."
            "\\end{titlepage}\n\n";

        static constexpr const char *workPages[] = {
            "\\section{\\Large{The actual work}}\n\n"
            "Let us, for example, differentiate:\n",

            "\n\n",
        };

        static constexpr const char *conclusionPages[] = {
            "\\section{\\Large{Conclusion}}\n"
            "So yeah, we got the derivative: \n"
            "$ \\left(",

            "\\right)'\\left(x\\right) = ",

            " $. \n"
            "The further simplification of the result, if at all possible, ""is left as an exercise to the reader.\n"
            "Now onto the real business: my payment. \\textbf{You owe me.}\n"
            "As a wise man once said, \\textit{\" \u2642 Differentiation is three hundred bucks \u2642 \"} ...\n",
        };

        static constexpr const char footer[] =
            "\\end{document}\n";

        // TODO: Merge?
        static constexpr const char *cmds[] = {
            "pdflatex -output-directory=exprs -jobname=log exprs/log.tex" /*" >nul"*/,
            "start exprs/log.pdf",
        };
    };

}

#endif // LATEX_CONSTS_H
