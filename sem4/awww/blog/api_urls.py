from django.urls import path

from pages import views
from . import api

urlpatterns = [
    path("posts/", api.PostListView.as_view(), name="api-post-list"),
    path("posts/<int:pk>/", api.PostDetailView.as_view(), name="api-post-detail"),
    path("posts/<int:pk>/comments/", api.PostCommentView.as_view()),
    path("posts/<int:post_pk>/comments/<int:comment_pk>/", api.CommentDetailView.as_view()),
]



#curl http://127.0.0.1:8000/api/posts/3/comments/