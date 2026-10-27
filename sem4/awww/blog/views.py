from django.http import HttpResponse, JsonResponse
from django.shortcuts import render, get_object_or_404, redirect
from .models import Post, Category, Comment
from .forms import CommentForm
from django.db.models import F
from django.db.models import Prefetch
from django.contrib.auth.decorators import login_required, user_passes_test
from .forms import PostForm
from django.db.models import Q


def post_list(request):
    posts = Post.objects.select_related("category")
    # Now: 1 JOIN query instead of N+1
    categories = Category.objects.all()
    return render(request, "blog/post_list.html", {
        "posts": posts, "categories": categories,
    })


def category_posts(request, slug):
    category = get_object_or_404(Category, slug=slug)
    posts = category.posts.all()
    return render(request, "blog/category_posts.html", {
        "category": category,
        "posts": posts,
        "categories": Category.objects.all(),  # needed for the sidebar
    })


   
def post_detail(request, slug):
    active_comments = Comment.objects.filter(active=True)
    post = get_object_or_404(
        Post.objects.prefetch_related(
            Prefetch("comments", queryset=active_comments, to_attr="active_comments")
        ),
        slug=slug,
    )
    
    Post.objects.filter(pk=post.pk).update(
        view_count=F("view_count") + 1
    )
    
    comments = post.active_comments   # already loaded, no extra query
    form     = CommentForm()

    if request.method == "POST":
        form = CommentForm(request.POST)
        if form.is_valid():
            comment      = form.save(commit=False)
            comment.post = post
            comment.save()

            return redirect("blog:post-detail", slug=post.slug)

    return render(request, "blog/post_detail.html", {
        "post": post,
        "comments": comments,
        "form": form,
    })
    
    
@login_required
@user_passes_test(lambda u: u.is_staff)
def post_create(request):

    if request.method == "POST":
        form = PostForm(request.POST)

        if form.is_valid():
            post = form.save()
            return redirect("blog:post-detail", slug=post.slug)

    else:
        form = PostForm()

    return render(request, "blog/post_form.html", {
        "form": form
    })
    
def blog_browser(request):
    return render(request, "pages/blog_browser.html")


