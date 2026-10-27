from django import forms
from .models import Comment
from .models import Post

class CommentForm(forms.ModelForm):
    class Meta:
        model  = Comment
        fields = ["author", "email", "body"]
        widgets = {
            "body": forms.Textarea(attrs={"rows": 4}),
        }
        labels = {
            "author": "Your Name",
        }

    def clean_author(self):
        author = self.cleaned_data["author"].strip()

        if len(author) < 2:
            raise forms.ValidationError("Name must be at least 2 characters long.")

        return author

    def clean_body(self):
        body = self.cleaned_data["body"]

        if len(body) > 1000:
            raise forms.ValidationError("Comment cannot be longer than 1000 characters.")

        return body
    
class PostForm(forms.ModelForm):
    class Meta:
        model = Post
        fields = ["title", "slug", "body", "category"]