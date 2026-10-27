from django.urls import path
from . import views
from .forms import PostForm

app_name = "blog"

urlpatterns = [
    path("", views.post_list, name="post-list"),
    path("post/<slug:slug>/", views.post_detail, name="post-detail"),
    path("category/<slug:slug>/", views.category_posts, name="category-posts"),
    path("new/", views.post_create, name="post-create"),
    path("browse/", views.blog_browser, name="blog-browser"),
]