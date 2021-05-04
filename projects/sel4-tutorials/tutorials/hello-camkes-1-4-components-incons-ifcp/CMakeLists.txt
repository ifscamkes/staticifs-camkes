cmake_minimum_required(VERSION 3.7.2)

project(hello-camkes-1 C)

DeclareTutorialsCAmkESComponent(A TEMPLATE_SOURCES components/A/src/a.c)
DeclareTutorialsCAmkESComponent(B TEMPLATE_SOURCES components/B/src/b.c)
DeclareTutorialsCAmkESComponent(C TEMPLATE_SOURCES components/C/src/c.c)
DeclareTutorialsCAmkESComponent(D TEMPLATE_SOURCES components/D/src/d.c)

DeclareTutorialsCAmkESRootserver(hello-camkes-1.camkes ADL_IS_TEMPLATE
    TEMPLATE_SOURCES interfaces/HelloSimple.camkes interfaces/HelloSimple1.camkes interfaces/HelloSimple2.camkes
    TUTORIAL_SOURCES components/A/A.camkes components/B/B.camkes components/C/C.camkes components/D/D.camkes
)