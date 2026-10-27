from django.urls import path, include
from . import views

urlpatterns = [
    path("", views.home, name="home"),
    path("about/", views.about, name="about"),
    path("greet/<str:name>/", views.greet, name="greet"),
    path("projects/", views.projects, name="projects"),
    path("guestbook/", views.guestbook, name="guestbook"),
    path("blog/", include("blog.urls")),
    path("demo/", views.api_demo, name="api-demo"),
]
