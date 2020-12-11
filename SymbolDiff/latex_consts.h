#ifndef LATEX_CONSTS_H
#define LATEX_CONSTS_H

namespace SymbolDiff {

    namespace LatexConsts {
        constexpr const char header[] =
            "\\documentclass[12pt]{article}\n"
            "\\usepackage[utf8]{inputenc}\n"
            "\\usepackage{textcomp}\n"
            "\\usepackage{scalerel}\n"
            "\\usepackage{newunicodechar}\n"
            "\\usepackage{graphicx}\n"
            "\\usepackage{indentfirst}\n"
            "\\usepackage{amsmath, nccmath}\n"
            "\\usepackage{hyperref}\n"
            "\\newunicodechar{\u2642}{\\scalerel*{\\includegraphics{log/u2642.png}}{\\textrm{" "\\#" /*"\\textbigcircle"*/ "}}}\n"
            "\\setlength{\\parskip}{1em}\n"
            "\\begin{document}\n"
            "\\begin{fleqn}[\\parindent]\n";

        constexpr const char *titlePages[] = {
            "\\begin{titlepage}"
            "\\begin{center}\\Large MIPT ICT, 2020\\end{center}"
            "\\begin{center}\\Large\\textit{Belyaev Andrey Alexeevich}\\end{center}"
            "\\begin{center}\\Huge\\textbf{\\underline{D I F F E R E N T I A T I O N}}\\end{center}"
            "\\begin{center}\\large\\textit{Patent pending}\\end{center}"
            "\\section{\\Large{Abstract}}\n"
            "So, scientific works get paid for, right? I've got a master plan... \n"
            "And in case the price is proportional to the word count, here's a little something for you:\\par\n"
            "\\textit{Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore "
            "et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut "
            "aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
            "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
            "culpa qui officia deserunt mollit anim id est laborum.}\\par\n"
            "The obvious only consequence of the abovestated is that we need to differentiate the so-called \\textit{formula ultima} (by %c): \\par\n",

            "\n"
            "\\end{titlepage}\n\n",
        };


        constexpr const char workPage[] =
            "\\section{\\Large{The actual work}}\n\n"
            "Let us then begin the differentiation. We shall use the well-known Belyaev's algorithm.\n";

        constexpr const char *conclusionPages[] = {
            "\\section{\\Large{Conclusion}}\n"
            "So yeah, we got the derivative: \n"
            "$ (",

            ")'(x) = ",

            " $. \n"
            "The further simplification of the result, if at all possible, ""is left as an exercise to the reader.\n"
            "Now onto the real business: my payment. \\textbf{You owe me.}\n"
            "As a wise man once said, \\textit{\" \u2642 Differentiation is three hundred bucks \u2642 \"} ...\n"
            "\\section{\\Large{References}}\n"
            "\\begin{enumerate}\n"
            "\\item Wikipedia. Lots of it. Approx. 40000 BC-2020 AD, I guess...\n"
            "\\item My glorious mind. Pages 371-378. 2002\n"
            "\\item A conspect book of Ilya Dedinsky's seminars, Andrew Belyaev, 2020\n"
            "\\item Jojo references, Dio Brando \\& many others, 1880-2000 + a couple of universe lifespan cycles\n"
            "\\item Linked list's cross-references (as a source of inspiration), God Almighty himself, exist in a separate concept space outside of the temporal continuum\n"
            "\\item Gachimuchi (in the conclusion), but I was serious: pay me!!!\n"
            "\\item \\url{https://github.com/abel1502/mipt_1s}, Andrew Belyaev again, 2020"
            "\\end{enumerate}\n",
        };

        constexpr const char footer[] =
            "\\end{fleqn}\n"
            "\\end{document}\n";

        // TODO: Merge?
        constexpr const char *cmds[] = {
            "pdflatex -output-directory=log -jobname=log log/log.tex" " >nul",
            "start log/log.pdf",
        };

        constexpr const char fileName[] = "log/log.tex";
    };

}

#endif // LATEX_CONSTS_H
